#include "pack/scripts/common.as"

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