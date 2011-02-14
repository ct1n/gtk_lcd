#include <stdio.h>
#include <gtk/gtk.h>
#include "gtk_lcd.h"

void double_to_lcd_str(double val, char *lstr)
{
    gchar buf[64];
    gchar *p;
    int i, dig;

    snprintf(buf, 64, "% 6.4lf", val);

    lstr[0] = buf[0];
    p = buf + 1;
    for (i = 0; i < 8; i += 2) {
        if (*p == '.') {
            lstr[i + 1] = '.';
            p++;
        }
        else {
            lstr[i + 1] = ' ';
        }
        lstr[i + 2] = *p;
        p++;
    }
    lstr[9] = 0;
    printf("%s\n", lstr);
}

void on_spinbutton_change(GtkSpinButton *spin, GtkLCD *lcd)
{
    gchar buf[16];
    double val;

    val = gtk_spin_button_get_value(spin);
    double_to_lcd_str(val, buf);
    gtk_lcd_set_value(lcd, buf);
}

int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *hbox;
    GtkWidget *lcd;
    GtkAdjustment *adj;
    GtkWidget *spin;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Sige");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 5);

    g_signal_connect(G_OBJECT(window), "destroy", 
                     G_CALLBACK(gtk_main_quit), NULL);

    hbox = gtk_hbox_new(0, 5);

    lcd = gtk_lcd_new("-.0.0.0.0");
    gtk_lcd_set_padding(GTK_LCD(lcd), 4);
    gtk_lcd_set_char_width(GTK_LCD(lcd), 18);
    gtk_lcd_set_char_height(GTK_LCD(lcd), 35);
    gtk_box_pack_start(GTK_BOX(hbox), lcd, 0, 0, 0);

    adj = (GtkAdjustment *) gtk_adjustment_new (145.425, 0.0, 1000.0, 0.01, 1.0, 0.0);
    spin = gtk_spin_button_new(adj, 1.0, 3);
    gtk_box_pack_start(GTK_BOX(hbox), spin, 1, 1, 0);

    gtk_container_add(GTK_CONTAINER(window), hbox);

    g_signal_connect(G_OBJECT(spin), "value_changed",
                     G_CALLBACK(on_spinbutton_change), lcd);

    on_spinbutton_change(spin, lcd);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

