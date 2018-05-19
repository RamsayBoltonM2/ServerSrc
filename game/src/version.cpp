#include <stdio.h>

void WriteVersion()
{
#ifndef __WIN32__
	FILE* fp = fopen("RamsayBolton.txt", "w");

	if (fp)
	{
		fprintf(fp, "DÜNYAM KARARDI SERVER FİLES © 2013-2023 Tüm Hakları Saklıdır.\n");
		//fprintf(fp, "%s@%s:%s\n", __USER__, __HOSTNAME__, __PWD__);
		fclose(fp);
	}
#endif
}

