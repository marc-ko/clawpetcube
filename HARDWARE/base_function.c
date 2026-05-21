#include "base_function.h"
#include "string.h"

void numToString(int num, char *str)
{
	uint8_t stack[40];
	int top = -1;
	memset(str, 0, sizeof(str));
	if (num < 0)
	{
		*(str++) = '-';
		num *= (-1);
	}
	if (num < 10)
	{
		str[0] = '0';
		str[1] = num + '0';
		return;
	}
	while (num != 0)
	{
		stack[++top] = num % 10;
		num /= 10;
	}
	while (top >= 0)
	{
		*str = (stack[top--] + '0');
		str++;
	}
}

int stringToNum(char *str)
{
	int i = 0;
	int ret = 0;
	while (str[i] != '\0')
	{
		ret = ret * 10;				// Multiply current value by 10 to shift digits left
		ret = ret + (str[i] - '0'); // Add new digit
		i++;
	}
	return ret;
}
