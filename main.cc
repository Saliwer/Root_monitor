#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <TROOT.h>
#include "TMyRint.h"

#define DESCRIPTION "ROOT Monitor"
#define VID "0.01"

extern void InitGui();

VoidFuncPtr_t initfuncs[] = { InitGui, 0 };

TROOT root("GUI", "GUI test environement");

/* Our main */
int main( int argc, char **argv )
{
  if(argc < 2)
    {
      printf("USAGE: %s <server_name:port_number>\n", argv[0]);
      return 0;
    }
  char app_name[32];
  strcpy(app_name,"App");
  TMyRint* theApp = new TMyRint(app_name,&argc,argv);
  theApp->Run();
  return 0;
} /* End of main */

void UpdateMonitorWindows()
{
  TMyRint::Instance()->FillHistograms();
  TMyRint::Instance()->UpdateHistograms();
}

void ResetHistograms()
{
  TMyRint::Instance()->ResetHistograms();
}

