#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include <sys/time.h>

#define mqtt_host "localhost"
#define mqtt_port 1883
#define TOPIC "sensors/temperature/data"
#define MESSAGE "{'sensor_id': 'temperature', 'value': '%d', timestamp:'%ld'}"

static int run = 1;

void
handle_signal (int s)
{
  run = 0;
}

void
connect_callback (struct mosquitto *mosq, void *obj, int result)
{
  printf ("connect callback, rc=%dn", result);
}

void
message_callback (struct mosquitto *mosq, void *obj,
		  const struct mosquitto_message *message)
{
  bool match = 0;
  printf ("got message '%.*s' for topic '%s'n", message->payloadlen,
	  (char *) message->payload, message->topic);
  mosquitto_topic_matches_sub ("hello", message->topic, &match);
  if (match)
    {
      printf ("got message for ADC topic");
    }
}

int
random_temperature ()
{
  return (int)((rand () % 10000) / 1000 + 23 );
}

double
random_number ()
{
  return rand ();
}

int
main (int argc, char *argv[])
{
  uint8_t reconnect = true;
  char clientid[24];
  struct mosquitto *mosq;
  struct timeval tv;
  int rc = 0;

  signal (SIGINT, handle_signal);
  signal (SIGTERM, handle_signal);

  mosquitto_lib_init ();

  memset (clientid, 0, 24);
  snprintf (clientid, 23, "mysql_log_%d", getpid ());
  mosq = mosquitto_new (clientid, true, 0);

  int ret = 0;
  char text[128];

  if (mosq)
    {
      // Setup connect and message callbacks
      mosquitto_connect_callback_set (mosq, connect_callback);
      mosquitto_message_callback_set (mosq, message_callback);

      // Connect to mosquitto broker
      rc = mosquitto_connect (mosq, mqtt_host, mqtt_port, 60);
      // mosquitto_subscribe (mosq, NULL, "hello", 0);

      while (run)
	{
	  // Run the main mosquitto loop and check for errors
	  // if an error occurs then reconnect
	  rc = mosquitto_loop (mosq, -1, 1);
	  if (run && rc)
	    {
	      printf ("connection error!n");
	      sleep (10);
	      mosquitto_reconnect (mosq);
	    }

	  gettimeofday (&tv, NULL);

	  sprintf (text, MESSAGE, random_temperature(), tv.tv_sec);
	  ret =
	    mosquitto_publish (mosq, NULL, TOPIC, strlen (text), text, 0,
			       false);
	  if (ret)
	    {
	      fprintf (stderr, "Can't publish to Mosquitto server");
	      rc = 0;
	    }
	  sleep (1);
	}
      mosquitto_destroy (mosq);
    }
  mosquitto_lib_cleanup ();
  return rc;
}
