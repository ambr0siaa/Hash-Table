#define BIL_IMPLEMENTATION
#include "bil.h"

int main(int argc, char **argv)
{
    BIL_REBUILD(argc, argv, BIL_CURRENT_DIR);
    int status = BIL_EXIT_SUCCESS;
    Bil_Cmd cmd = {0};

bil_workflow_begin();
    
    bil_cmd_append(&cmd, "gcc");
    bil_cmd_append(&cmd, "-lm", "-Wall", "-Wextra", "-flto", "-Ofast", "-fPIE");
    bil_cmd_append(&cmd, "-o", "table");
    bil_cmd_append(&cmd, "table.c");

    if (!bil_cmd_run_sync(&cmd))
        status = BIL_EXIT_FAILURE;

bil_workflow_end();

    return status;
}
