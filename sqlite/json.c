/*
 * A simple example of json string parsing with json-c.
 *
 * clang -Wall -g -I/usr/include/json-c/ -o json_parser json_parser.c -ljson-c
 */
#include <json-c/json.h>
#include <stdio.h>

int main() {
	struct json_object *jobj;
	char *str = "{ \"sensor_id\": \"temperature\", \
								 \"value\": \"23\", \
								 \"timestamp\": \"1481747234\"}";

	printf("str:\n---\n%s\n---\n\n", str);

	jobj = json_tokener_parse(str);
	printf("jobj from str:\n---\n%s\n---\n", json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));

	return 0;
}
