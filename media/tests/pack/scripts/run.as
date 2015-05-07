#include "pack/scripts/common.as"

int counter = 0;

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
	counter++;
	
	if(counter > 30)
		breakRender();
} 