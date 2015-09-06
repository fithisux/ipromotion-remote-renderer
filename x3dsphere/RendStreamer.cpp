/*
===========================================================================
IPROMOTION REMOTE RENDERER Source Code
This file is part of the IPROMOTION REMOTE RENDERER Source Code (?IPROMOTION REMOTE RENDERE Source Code?).  
IPROMOTION REMOTE RENDERE Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
IPROMOTION REMOTE RENDERE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with IPROMOTION REMOTE RENDERE Source Code.  If not, see <http://www.gnu.org/licenses/>.
In addition, the IPROMOTION REMOTE RENDERE Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.
If you have questions concerning this license or the applicable additional terms, you may contact in writing Vasileios Anagnostopoulos, Campani 3 Street, Athens Greece, POBOX 11252.

Dr. Vasileios Anagnostopoulos is the developer of the initial code under project Ipromotion.
===========================================================================
*/
#include "RendStreamer.h"
#include <string.h>
#include <gst/gst.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>

typedef struct
{
  gchar *name;
  GSocketConnection *connection;
  GSocket *socket;
  GInputStream *istream;
  GOutputStream *ostream;
  GSource *isource, *tosource;
  GByteArray *current_message;
} Client;

static GMainLoop *loop = NULL;
G_LOCK_DEFINE_STATIC (clients);
static GList *clients = NULL;
static GstElement *multisocketsink = NULL;
static gboolean started = FALSE;
GstElement  *pipeline;

static void
remove_client (Client * client)
{
  g_print ("Removing connection %s\n", client->name);

  g_free (client->name);

  if (client->isource) {
    g_source_destroy (client->isource);
    g_source_unref (client->isource);
  }
  if (client->tosource) {
    g_source_destroy (client->tosource);
    g_source_unref (client->tosource);
  }
  g_object_unref (client->connection);
  g_byte_array_unref (client->current_message);

  G_LOCK (clients);
  clients = g_list_remove (clients, client);
  G_UNLOCK (clients);

  g_slice_free (Client, client);
}

static void
write_bytes (Client * client, const gchar * data, guint len)
{
  gssize w;
  GError *err = NULL;

  /* TODO: We assume this never blocks */
  do {
    w = g_output_stream_write (client->ostream, data, len, NULL, &err);
    if (w > 0) {
      len -= w;
      data += w;
    }
  } while (w > 0 && len > 0);

  if (w <= 0) {
    if (err) {
      g_print ("Write error %s\n", err->message);
      g_clear_error (&err);
    }
    remove_client (client);
  }
}

static void
client_message (Client * client, const gchar * data, guint len)
{
  gchar **lines = g_strsplit_set (data, "\r\n", -1);

  if (g_str_has_prefix (lines[0], "HEAD")) {
    gchar **parts = g_strsplit (lines[0], " ", -1);
    gchar *response;
    const gchar *http_version;

    if (parts[1] && parts[2] && *parts[2] != '\0')
      http_version = parts[2];
    else
      http_version = "HTTP/1.0";

    if (parts[1] && strcmp (parts[1], "/") == 0) {
      response = g_strdup_printf ("%s 200 OK\r\n" "\r\n", http_version);
    } else {
      response = g_strdup_printf ("%s 404 Not Found\r\n\r\n", http_version);
    }
    write_bytes (client, response, strlen (response));
    g_free (response);
    g_strfreev (parts);
  } else if (g_str_has_prefix (lines[0], "GET")) {
    gchar **parts = g_strsplit (lines[0], " ", -1);
    gchar *response;
    const gchar *http_version;
    gboolean ok = FALSE;

    if (parts[1] && parts[2] && *parts[2] != '\0')
      http_version = parts[2];
    else
      http_version = "HTTP/1.0";

    if (parts[1] && strcmp (parts[1], "/") == 0) {
      response = g_strdup_printf ("%s 200 OK\r\n" "\r\n", http_version);
      ok = TRUE;
    } else {
      response = g_strdup_printf ("%s 404 Not Found\r\n\r\n", http_version);
    }
    write_bytes (client, response, strlen (response));
    g_free (response);
    g_strfreev (parts);

    if (ok) {
      g_source_destroy (client->isource);
      g_source_unref (client->isource);
      client->isource = NULL;
      g_source_destroy (client->tosource);
      g_source_unref (client->tosource);
      client->tosource = NULL;
      g_print ("Starting to stream to %s\n", client->name);
      g_signal_emit_by_name (multisocketsink, "add", client->socket);

      if (!started) {
        g_print ("Starting pipeline\n");
        if (gst_element_set_state (pipeline,
                GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
          g_print ("Failed to start pipeline\n");
          g_main_loop_quit (loop);
        }
        started = TRUE;
      }
    }
  } else {
    gchar **parts = g_strsplit (lines[0], " ", -1);
    gchar *response;
    const gchar *http_version;

    if (parts[1] && parts[2] && *parts[2] != '\0')
      http_version = parts[2];
    else
      http_version = "HTTP/1.0";

    response = g_strdup_printf ("%s 400 Bad Request\r\n\r\n", http_version);
    write_bytes (client, response, strlen (response));
    g_free (response);
    g_strfreev (parts);
    remove_client (client);
  }

  g_strfreev (lines);
}

static gboolean
on_timeout (Client * client)
{
  g_print ("Timeout\n");
  remove_client (client);

  return FALSE;
}

static gboolean
on_read_bytes (GPollableInputStream * stream, Client * client)
{
  gssize r;
  gchar data[4096];
  GError *err = NULL;

  do {
    r = g_pollable_input_stream_read_nonblocking (G_POLLABLE_INPUT_STREAM
        (client->istream), data, sizeof (data), NULL, &err);
    if (r > 0)
      g_byte_array_append (client->current_message, (guint8 *) data, r);
  } while (r > 0);

  if (r == 0) {
    remove_client (client);
    return FALSE;
  } else if (g_error_matches (err, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) {
    guint8 *tmp = client->current_message->data;

    g_clear_error (&err);

    while (client->current_message->len > 3) {
      if (tmp[0] == 0x0d && tmp[1] == 0x0a && tmp[2] == 0x0d && tmp[3] == 0x0a) {
        guint len;

        g_byte_array_append (client->current_message, (const guint8 *) "\0", 1);
        len = tmp - client->current_message->data + 5;
        client_message (client, (gchar *) client->current_message->data, len);
        g_byte_array_remove_range (client->current_message, 0, len);
        tmp = client->current_message->data;
      } else {
        tmp++;
      }
    }

    if (client->current_message->len >= 1024 * 1024) {
      g_print ("No complete request after 1MB of data\n");
      remove_client (client);
      return FALSE;
    }

    return TRUE;
  } else {
    g_print ("Read error %s\n", err->message);
    g_clear_error (&err);
    remove_client (client);
    return FALSE;
  }

  return FALSE;
}

static gboolean
on_new_connection (GSocketService * service, GSocketConnection * connection,
    GObject * source_object, gpointer user_data)
{
  Client *client = g_slice_new0 (Client);
  GSocketAddress *addr;
  GInetAddress *iaddr;
  gchar *ip;
  guint16 port;

  addr = g_socket_connection_get_remote_address (connection, NULL);
  iaddr = g_inet_socket_address_get_address (G_INET_SOCKET_ADDRESS (addr));
  port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (addr));
  ip = g_inet_address_to_string (iaddr);
  client->name = g_strdup_printf ("%s:%u", ip, port);
  g_free (ip);
  g_object_unref (addr);

  g_print ("New connection %s\n", client->name);

  client->connection = (GSocketConnection *) g_object_ref (connection);
  client->socket = g_socket_connection_get_socket (connection);
  client->istream =
      g_io_stream_get_input_stream (G_IO_STREAM (client->connection));
  client->ostream =
      g_io_stream_get_output_stream (G_IO_STREAM (client->connection));
  client->current_message = g_byte_array_sized_new (1024);

  client->tosource = g_timeout_source_new_seconds (5);
  g_source_set_callback (client->tosource, (GSourceFunc) on_timeout, client,
      NULL);
  g_source_attach (client->tosource, NULL);

  client->isource =
      g_pollable_input_stream_create_source (G_POLLABLE_INPUT_STREAM
      (client->istream), NULL);
  g_source_set_callback (client->isource, (GSourceFunc) on_read_bytes, client,
      NULL);
  g_source_attach (client->isource, NULL);

  G_LOCK (clients);
  clients = g_list_prepend (clients, client);
  G_UNLOCK (clients);

  return TRUE;
}

static gboolean
on_message (GstBus * bus, GstMessage * message, gpointer user_data)
{
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      gchar *debug;
      GError *err;

      gst_message_parse_error (message, &err, &debug);
      g_print ("Error %s\n", err->message);
      g_error_free (err);
      g_free (debug);
      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_WARNING:{
      gchar *debug;
      GError *err;

      gst_message_parse_error (message, &err, &debug);
      g_print ("Warning %s\n", err->message);
      g_error_free (err);
      g_free (debug);
      break;
    }
    case GST_MESSAGE_EOS:{
      g_print ("EOS\n");
      g_main_loop_quit (loop);
    }
    default:
      break;
  }

  return TRUE;
}

static void
on_client_socket_removed (GstElement * element, GSocket * socket,
    gpointer user_data)
{
  GList *l;
  Client *client = NULL;

  G_LOCK (clients);
  for (l = clients; l; l = l->next) {
    Client *tmp = (Client *) l->data;
    if (socket == tmp->socket) {
      client = tmp;
      break;
    }
  }
  G_UNLOCK (clients);

  if (client)
    remove_client (client);
}


RendStreamer::RendStreamer(promoconf * myconf,wqueue<WorkItem*>* myqueue) :
    pconf(myconf),frame_queue(myqueue)
    {    
        printf("construct\n");
    }

    void* RendStreamer::run()
    {

        cout << "rendstreamer started " << endl;
        gchar * description=(gchar *) g_malloc(1000);
        g_sprintf(description,
//"webmmux name=stream appsrc name=mysrc ! video/x-raw,format=RGB,width=%d,height=%d,framerate=20/1 ! videoconvert ! vp8enc !  stream.",
"webmmux name=stream appsrc name=mysrc ! video/x-raw,format=RGB,width=%d,height=%d,framerate=5/1 ! videoconvert ! vp8enc  !  stream.",  
                this->pconf->width,this->pconf->height);
        gint argc=0;
        gst_init (&argc, NULL);

        GError *err = NULL;
        GstElement * bin = gst_parse_launch (description, &err);

        g_free(description);

          if ( (!bin) || (err)) {
            g_print ("invalid pipeline: %s\n", err->message);
            g_clear_error (&err);
            exit(-2);
          }

          multisocketsink = gst_element_factory_make ("multisocketsink", NULL);


 printf("parsed %d, %d\n",this->pconf->width,this->pconf->height);
          /* setup appsrc */
          GstElement *appsrc = gst_bin_get_by_name (GST_BIN (bin), "mysrc");
          if (!appsrc) {
            g_print ("no element with name \"appsrc\" found\n");
            gst_object_unref (bin);
            exit(-3);
          }
          /*
          g_object_set (G_OBJECT (appsrc), "caps",
                gst_caps_new_simple ("video/x-raw",
                             "format", G_TYPE_STRING, "RGB",
                             "width", G_TYPE_INT, this->width,
                             "height", G_TYPE_INT, this->height,
                             "framerate", GST_TYPE_FRACTION, 1, 2,
                             NULL), NULL);


    */

     gst_app_src_set_size (GST_APP_SRC (appsrc), (gint64) -1); // total stream size is not known
     gst_app_src_set_stream_type(GST_APP_SRC (appsrc),GST_APP_STREAM_TYPE_STREAM);
     g_object_set (G_OBJECT (appsrc),"format", GST_FORMAT_TIME, NULL);


            /*setup muxer*/
            GstElement *stream = gst_bin_get_by_name (GST_BIN (bin), "stream");
              if (!stream) {
                g_print ("no element with name \"stream\" found\n");
                gst_object_unref (bin);
                exit(-3);
              }

              GstPad *srcpad = gst_element_get_static_pad (stream, "src");
              if (!srcpad) {
                g_print ("no \"src\" pad in element \"stream\" found\n");
                gst_object_unref (stream);
                gst_object_unref (bin);
                exit(-4);
              }

              GstPad *ghostpad = gst_ghost_pad_new ("src", srcpad);
              gst_element_add_pad (GST_ELEMENT (bin), ghostpad);
              gst_object_unref (srcpad);


         /* add to pipeline */
          pipeline = gst_pipeline_new ("pipeline");
          gst_bin_add_many (GST_BIN (pipeline), bin,multisocketsink, NULL);

          /* link with multi socket */
          GstPad *sinkpad = gst_element_get_static_pad (multisocketsink, "sink");
          gst_pad_link (ghostpad, sinkpad);
          gst_object_unref (sinkpad);

          /*get the bus */
          GstBus *bus = gst_element_get_bus (pipeline);
          gst_bus_add_signal_watch (bus);
          g_signal_connect (bus, "message", G_CALLBACK (on_message), NULL);
          gst_object_unref (bus);

            /*call backs */
          g_signal_connect (multisocketsink, "client-socket-removed",G_CALLBACK (on_client_socket_removed), NULL);
          g_signal_connect (appsrc, "need-data", G_CALLBACK (cb_need_data), this);

          /* setup main loop */
          loop = g_main_loop_new (NULL, FALSE);
          if (gst_element_set_state (pipeline,GST_STATE_READY) == GST_STATE_CHANGE_FAILURE) {
                    gst_object_unref (pipeline);
                    g_main_loop_unref (loop);
                    g_print ("Failed to set pipeline to ready\n");
                    exit(-5);
          }

          /*setup server*/
          service = g_socket_service_new ();
          g_socket_listener_add_inet_port (G_SOCKET_LISTENER (service), this->pconf->port, NULL,NULL);
          g_signal_connect (service, "incoming", G_CALLBACK (on_new_connection), NULL);
          g_socket_service_start (service);
          g_print ("Listening on http://127.0.0.1:%u/\n",this->pconf->port);

        /* start main loop */
        g_main_loop_run (loop);
        cout << "Fuck " << endl;
        return NULL;
    }


      RendStreamer::~RendStreamer(){
          g_socket_service_stop (this->service);
          g_object_unref (this->service);
          gst_element_set_state (pipeline, GST_STATE_NULL);
          gst_object_unref (pipeline);
          g_main_loop_unref (this->loop);          
      }


     void
cb_need_data (GstElement *appsrc,
	      guint       unused_size,
	      RendStreamer*    rendo)
{
  static GstClockTime timestamp = 0;
  GstFlowReturn ret;

//    printf("cb called \n");;
    WorkItem * item=rendo->frame_queue->remove();
    GstBuffer *buffer=gst_buffer_new_wrapped (item->buffer,item->bytes);
    item->buffer=NULL;
    delete item;

  GST_BUFFER_PTS (buffer) = timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 20);

  timestamp += GST_BUFFER_DURATION (buffer);

//  printf("pushing the buffer \n");;
  g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
//  printf("pushed the buffer \n");;

gst_buffer_unref(buffer);
  if (ret != GST_FLOW_OK) {
    /* something wrong, stop pushing */
    g_main_loop_quit (rendo->loop);
  }
}
