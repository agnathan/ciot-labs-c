#include <json-c/json.h>
#include <stdio.h>
#include <string.h>

void
json_parse (json_object * jobj)
{
  char sensor_id[24];
  char value[24];
  char timestamp[24];
  enum json_type type;

  json_object_object_foreach (jobj, key, val)
  {
    type = json_object_get_type (val);
    switch (type)
      {
        case json_type_int:
          printf ("%s:%d\n", key, json_object_get_int (val));
          break;
        case json_type_string:
          if (0 == strcmp(key, "sensor_id")) {
            memset (sensor_id, 0, 24);
            snprintf (sensor_id, 23, "%s", json_object_get_string(val));
          }

          if (0 == strcmp(key, "value")) {
            memset (value, 0, 24);
            snprintf (value, 23, "%s", json_object_get_string(val));
          }

          if (0 == strcmp(key, "timestamp")) {
            memset (timestamp, 0, 24);
            snprintf (timestamp, 23, "%s", json_object_get_string(val));
          }

          printf ("%s:%s\n", key, json_object_get_string (val));
          break;
      }
  }
}

int
main ()
{
  char *string = "{ \"sensor_id\" : \"temperature\", \"value\" : \"7\", \"timestamp\" : \"1481747233\" }";
  printf ("JSON string: %s\n", string);
  json_object *jobj = json_tokener_parse (string);
  json_parse (jobj);
}
