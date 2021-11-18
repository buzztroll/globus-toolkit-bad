
#include "globus_stdio_ui.h"
#include "openssl/ui.h"

static
int
globus_l_stdio_ui_activate(void);

static
int
globus_l_stdio_ui_deactivate(void);

static
int
globus_l_stdio_ui_read(
    UI                                 *ui,
    UI_STRING                          *uis);

static
int
globus_l_stdio_ui_write(
    UI                                 *ui,
    UI_STRING                          *uis);

globus_module_descriptor_t              globus_i_stdio_ui_module =
{
    "globus_stdio_ui",
    globus_l_stdio_ui_activate,
    globus_l_stdio_ui_deactivate,
};

static const UI_METHOD                 *globus_l_stdio_ui_old_method = NULL;
static UI_METHOD                       *globus_l_stdio_ui_method = NULL;

int
globus_l_stdio_ui_activate(void)
{
    globus_l_stdio_ui_method = UI_create_method("globus_stdio_ui");
    globus_assert(globus_l_stdio_ui_method != NULL);
    UI_method_set_reader(globus_l_stdio_ui_method, globus_l_stdio_ui_read);
    UI_method_set_writer(globus_l_stdio_ui_method, globus_l_stdio_ui_write);
    globus_l_stdio_ui_old_method = UI_get_default_method();
    UI_set_default_method(globus_l_stdio_ui_method);

    return 0;
}
/* globus_l_stdio_ui_activate() */

int
globus_l_stdio_ui_deactivate(void)
{
    globus_assert(globus_l_stdio_ui_method != NULL);
    UI_set_default_method(globus_l_stdio_ui_old_method);
    UI_destroy_method(globus_l_stdio_ui_method);

    return 0;
}
/* globus_l_stdio_ui_deactivate() */

static
int
globus_l_stdio_ui_write(
    UI                                 *ui,
    UI_STRING                          *uis)
{
    printf("%s", UI_get0_output_string(uis));

    return 1;
}
/* globus_l_stdio_ui_write() */

static
int
globus_l_stdio_ui_read(
    UI                                 *ui,
    UI_STRING                          *uis)
{
    char                                buf[100], *res;

    res = fgets(buf, sizeof(buf), stdin);

    if (res == NULL)
    {
        return 0;
    }
    if (res[strlen(res)-1] == '\n')
    {
        res[strlen(res)-1] = 0;
    }

    return UI_set_result(ui, uis, res);
}
/* globus_l_stdio_ui_read() */
