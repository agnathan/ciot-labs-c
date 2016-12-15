#include <stdio.h>
#include <string.h>

int main ()
{
   const char str[] = "{'sensor_id': 'temperature', 'value': '23', timestamp:'1481747239'}";
   const char ch = ':';
   char *ret;

   ret = strchr(str, ch);
   ret = strchr(ret, '\'');

   printf("String after |%c| is - |%s|\n", ch, ret);

   return(0);
}
