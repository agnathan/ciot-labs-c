#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include <sys/time.h>
#include <sqlite3.h>
#include <json-c/json.h>

#define mqtt_host "localhost"
#define mqtt_port 1883
#define TOPIC "sensors/temperature/data"
#define MESSAGE "{'sensor_id': 'temperature', 'value': '%d', timestamp:'%ld'}"

static int run = 1;

sqlite3 *db;
char *err_msg = 0;
int sqlrc;

void
handle_signal (int s)
{
  run = 0;
}

void
connect_callback (struct mosquitto *mosq, void *obj, int result)
{
  printf ("connect callback, rc=%d\n", result);
}

void
message_callback (struct mosquitto *mosq, void *obj,
		  const struct mosquitto_message *message)
{
  bool match = 0;
  char sensor_id[24];
  char value[24];
  char timestamp[24];
  char sql[256];
  enum json_type type;

  printf ("%s : %.*s\n", message->topic, message->payloadlen,
	  (char *) message->payload);

  json_object *jobj = json_tokener_parse ((char *) message->payload);

  json_object_object_foreach (jobj, key, val)
  {
    type = json_object_get_type (val);
    switch (type)
      {
      case json_type_string:
	if (0 == strcmp (key, "sensor_id"))
	  {
	    memset (sensor_id, 0, 24);
	    snprintf (sensor_id, 23, "%s", json_object_get_string (val));
	  }

	if (0 == strcmp (key, "value"))
	  {
	    memset (value, 0, 24);
	    snprintf (value, 23, "%s", json_object_get_string (val));
	  }

	if (0 == strcmp (key, "timestamp"))
	  {
	    memset (timestamp, 0, 24);
	    snprintf (timestamp, 23, "%s", json_object_get_string (val));
	  }

	break;
      }
  }

  memset (sql, 0, 256);
  snprintf (sql, 255, "INSERT INTO sensordata VALUES('%s', '%s', '%s')",
	    sensor_id, value, timestamp);
  printf ("%s\n", sql);

  sqlrc = sqlite3_exec (db, sql, 0, 0, &err_msg);

  if (sqlrc != SQLITE_OK)
    {

      fprintf (stderr, "SQL error: %s\n", err_msg);
      run = 0;
    }

  // mosquitto_topic_matches_sub (TOPIC, message->topic, &match);
  // if (match)
  //   {
  //     printf ("got message for ADC topic\n");
  //   }
}

int
random_temperature ()
{
  return (int) ((rand () % 10000) / 1000 + 23);
}

int
main (int argc, char *argv[])
{
  uint8_t reconnect = true;
  char clientid[24];
  struct mosquitto *mosq;
  struct timeval tv;
  int rc = 0;

  // Setup signal handlers
  signal (SIGINT, handle_signal);
  signal (SIGTERM, handle_signal);

  // Initalize mosquitto library
  mosquitto_lib_init ();

  memset (clientid, 0, 24);
  snprintf (clientid, 23, "mysql_log_%d", getpid ());
  mosq = mosquitto_new (clientid, true, 0);

  // Initalize SQLite connection
  sqlrc = sqlite3_open ("data.db", &db);

  if (sqlrc != SQLITE_OK)
    {
      fprintf (stderr, "Cannot open database: %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);

      return 1;
    }

  char *sql = "DROP TABLE IF EXISTS sensordata;"
    "CREATE TABLE sensordata(sensor_id TEXT, value TEXT, timestamp INT);";

  sqlrc = sqlite3_exec (db, sql, 0, 0, &err_msg);

  int ret = 0;
  char text[128];

  if (mosq)
    {
      // Setup connect and message callbacks
      mosquitto_connect_callback_set (mosq, connect_callback);
      mosquitto_message_callback_set (mosq, message_callback);

      // Connect to mosquitto broker
      rc = mosquitto_connect (mosq, mqtt_host, mqtt_port, 60);
      mosquitto_subscribe (mosq, NULL, TOPIC, 0);

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
	  sleep (1);
	}
      mosquitto_destroy (mosq);
    }

  if (sqlrc != SQLITE_OK)
    {

      fprintf (stderr, "SQL error: %s\n", err_msg);

      sqlite3_free (err_msg);
      sqlite3_close (db);

      return 1;
    }

  sqlite3_close (db);

  mosquitto_lib_cleanup ();
  return rc;
}
