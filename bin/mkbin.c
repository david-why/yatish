#include <stdio.h>

int main()
{
    FILE *f = fopen(OUTFILE, "wb");
    printf("Data length: %ld\n", sizeof(OUTDATA));
    fwrite(OUTDATA, sizeof(OUTDATA), 1, f);
    fclose(f);
    return 0;
}
