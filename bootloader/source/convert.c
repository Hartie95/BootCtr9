 #include "convert.h"

int myAtoi(const char *str)
{
    int res = 0; // Initialize result
  	int startvalue = 0;
    // Iterate through all characters of input string and
    // update result
    bool isnegativ=false;
    if(str[0]=='-')
    {
    	isnegativ=true;
    	startvalue=1;
    }
    for (int i = startvalue; str[i] != '\0'; ++i)
    {
        if(str[i] >= '0' && str[i] <= '9')
        {
            res = res*10 + str[i] - '0';
        }
        else
            break;
    } 
  
  	if(isnegativ)
  		res*=-1;
    
    return res;
}
int chartoint(int c)
{
    char hex[] = "aAbBcCdDeEfF";
    int i;
    int result = 0;

    for(i = 0; result == 0 && hex[i] != '\0'; i++)
    {
        if(hex[i] == c)
        {
            result = 10 + (i / 2);
        }
    }

    return result;
}

unsigned int htoi(const char s[])
{
    unsigned int result = 0;
    int i = 0;
    int temp;

    //To take care of 0x and 0X added before the hex no.
    if(s[i] == '0')
    {
        ++i;
        if(s[i] == 'x' || s[i] == 'X')
        {
            ++i;
        }
    }

    while(s[i] != '\0')
    {
        result = result * 16;
        if(s[i] >= '0' && s[i] <= '9')
        {
            result = result + (s[i] - '0');
        }
        else
        {
            temp = chartoint(s[i]);
            if(!temp)
            {
                //If any character is not a proper hex no. ,  return 0
                return 0;
            }
            result = result + temp;
        }

        ++i;
    }

    return result;
}

int numberToInt(const char* value)
{
	if(value[0]=='0'&& (value[1]=='x'||value[1]=='X'))
		return htoi(value);
	else
		return myAtoi(value);
}