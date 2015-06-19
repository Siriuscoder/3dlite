#include "filesystem_01:pack/scripts/common.as"

int counter = 15;

void init()
{
    int i = 0;
    i = sum(i++, --i);
}

void shut()
{
    int b = 0;
    b = sum(sub(b, 4), 10);
}

void fixedUpdate()
{
	counter = sub(counter, 1);
	
	logInfo("fixedUpdate: in " + formatInt(counter, "") + " times");
	if(counter <= 0)
		breakRender();
} 