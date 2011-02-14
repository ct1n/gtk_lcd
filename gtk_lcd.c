#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "gtk_lcd.h"

static void gtk_lcd_class_init(GtkLCDClass *klass);
static void gtk_lcd_init(GtkLCD *lcd);
static void gtk_lcd_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void gtk_lcd_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void gtk_lcd_realize(GtkWidget *widget);
static gboolean gtk_lcd_expose(GtkWidget *widget, GdkEventExpose *event);
static void gtk_lcd_paint(GtkWidget *widget);
static void gtk_lcd_destroy(GtkObject *object);


GtkType gtk_lcd_get_type(void)
{
	static const GtkTypeInfo gtk_lcd_info = {
	    "GtkLCD",
	    sizeof(GtkLCD),
	    sizeof(GtkLCDClass),
	    (GtkClassInitFunc) gtk_lcd_class_init,
	    (GtkObjectInitFunc) gtk_lcd_init,
	    NULL,
	    NULL,
	    (GtkClassInitFunc) NULL
	};
    static GtkType gtk_lcd_type = 0;


    if (!gtk_lcd_type) {
        gtk_lcd_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_lcd_info);
    }


    return gtk_lcd_type;
}

void gtk_lcd_set_format(GtkLCD *lcd, const gchar *format)
{
    g_free(lcd->format);
    g_free(lcd->value);
    lcd->format = g_strdup(format);
    lcd->length = strlen(format);
    lcd->value = g_strnfill(lcd->length, ' ');
    gtk_widget_queue_resize(GTK_WIDGET(lcd));
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_value(GtkLCD *lcd, const gchar *value)
{
    g_strlcpy(lcd->value, value, lcd->length + 1);
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_padding(GtkLCD *lcd, gint val)
{
    lcd->padding = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_char_height(GtkLCD *lcd, gint val)
{
    lcd->char_height = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_char_width(GtkLCD *lcd, gint val)
{
    lcd->char_width = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_dot_width(GtkLCD *lcd, gint val)
{
    lcd->dot_width = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_sign_width(GtkLCD *lcd, gint val)
{
    lcd->sign_width = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_space_width(GtkLCD *lcd, gint val)
{
    lcd->space_width = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_line_thickness(GtkLCD *lcd, gint val)
{
    lcd->line_thickness = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_fg(GtkLCD *lcd, double red, double green, double blue)
{
    lcd->fg.red = red;
    lcd->fg.green = green;
    lcd->fg.blue = blue;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_bg(GtkLCD *lcd, double red, double green, double blue)
{
    lcd->bg.red = red;
    lcd->bg.green = green;
    lcd->bg.blue = blue;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

GtkWidget *gtk_lcd_new(const gchar *format)
{
    GtkLCD *lcd;

    lcd = gtk_type_new(gtk_lcd_get_type());
    if (format && format[0]) {
        gtk_lcd_set_format(lcd, format);
    }

    return GTK_WIDGET(lcd);
}

static void gtk_lcd_class_init(GtkLCDClass *klass)
{
    GtkWidgetClass *widget_class;
    GtkObjectClass *object_class;

    widget_class = (GtkWidgetClass *) klass;
    object_class = (GtkObjectClass *) klass;

    widget_class->realize = gtk_lcd_realize;
    widget_class->size_request = gtk_lcd_size_request;
    widget_class->size_allocate = gtk_lcd_size_allocate;
    widget_class->expose_event = gtk_lcd_expose;

    object_class->destroy = gtk_lcd_destroy;
}


static void gtk_lcd_init(GtkLCD *lcd)
{
    lcd->length = 0;
    lcd->format = NULL;
    lcd->value = NULL;
    
    lcd->padding = 2;
    lcd->char_height = 27;
    lcd->char_width = 12;
    lcd->dot_width = 3;
    lcd->sign_width = 6;
    lcd->space_width = 2;
    lcd->line_thickness = 3;

    lcd->fg.red = 0.6;
    lcd->fg.green = 1.0;
    lcd->fg.blue = 0.0;

    lcd->bg.red = 0.0;
    lcd->bg.green = 0.0;
    lcd->bg.blue = 0.0;
}


static void gtk_lcd_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    GtkLCD *lcd;
    gint width;
    gint height;
    gchar *p;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_LCD(widget));
    g_return_if_fail(requisition != NULL);

    lcd = GTK_LCD(widget);

    width = lcd->padding * 2 + lcd->space_width;
    height = lcd->padding * 2 + lcd->char_height + lcd->space_width * 2;

    for (p = lcd->format; *p; p++) {
        switch (*p) {
            case '-':
            case '+':
                width += lcd->sign_width;
                break;
            case '0':
            case ' ':
                width += lcd->char_width;
                break;
            case '.':
            case ':':
                width += lcd->dot_width;
                break;
        }
        width += lcd->space_width;
    }

    requisition->width = width;
    requisition->height = height;
}


static void gtk_lcd_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_LCD(widget));
    g_return_if_fail(allocation != NULL);

    widget->allocation = *allocation;

    if (GTK_WIDGET_REALIZED(widget)) {
        gdk_window_move_resize(widget->window,
                               allocation->x, allocation->y,
                               allocation->width, allocation->height);
    }
}

static void gtk_lcd_realize(GtkWidget *widget)
{
    GdkWindowAttr attributes;
    guint attributes_mask;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_LCD(widget));

    GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;

    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y;

    widget->window = gdk_window_new(gtk_widget_get_parent_window(widget),
				                    &attributes, attributes_mask);

    gdk_window_set_user_data(widget->window, widget);

    widget->style = gtk_style_attach(widget->style, widget->window);
    gtk_style_set_background(widget->style, widget->window,
			                 GTK_STATE_NORMAL);
}


static gboolean gtk_lcd_expose(GtkWidget *widget, GdkEventExpose *event)
{
    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_LCD(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    gtk_lcd_paint(widget);

    return FALSE;
}

static void paint_sign(GtkLCD *lcd, cairo_t *cr, gint offset, gchar ch)
{
    double x, y, len;

    switch (ch) {
        case '-':
            y = lcd->padding + lcd->space_width;

            x = offset;
            x += ((double) lcd->line_thickness) / 2;
            y += ((double) lcd->char_height) / 2;
            len = lcd->sign_width - lcd->line_thickness;
            cairo_move_to(cr, x, y);
            cairo_line_to(cr, x + len, y);
            cairo_set_line_width(cr, lcd->line_thickness);
            cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
            cairo_stroke(cr);
            break;
    }
}

static void paint_digit(GtkLCD *lcd, cairo_t *cr, gint offset, gchar ch)
{
    gint leds[7];
    gint i;
    double half_thick;
    gint line_width;
    gint line_height;
    double x, y, len;

    switch (ch) {
        case ' ': leds[0] = 0; leds[1] = 0; leds[2] = 0; leds[3] = 0; leds[4] = 0; leds[5] = 0; leds[6] = 0; break;
        case '0': leds[0] = 1; leds[1] = 1; leds[2] = 1; leds[3] = 0; leds[4] = 1; leds[5] = 1; leds[6] = 1; break;
        case '1': leds[0] = 0; leds[1] = 0; leds[2] = 1; leds[3] = 0; leds[4] = 0; leds[5] = 1; leds[6] = 0; break;
        case '2': leds[0] = 1; leds[1] = 0; leds[2] = 1; leds[3] = 1; leds[4] = 1; leds[5] = 0; leds[6] = 1; break;
        case '3': leds[0] = 1; leds[1] = 0; leds[2] = 1; leds[3] = 1; leds[4] = 0; leds[5] = 1; leds[6] = 1; break;
        case '4': leds[0] = 0; leds[1] = 1; leds[2] = 1; leds[3] = 1; leds[4] = 0; leds[5] = 1; leds[6] = 0; break;
        case '5': leds[0] = 1; leds[1] = 1; leds[2] = 0; leds[3] = 1; leds[4] = 0; leds[5] = 1; leds[6] = 1; break;
        case '6': leds[0] = 1; leds[1] = 1; leds[2] = 0; leds[3] = 1; leds[4] = 1; leds[5] = 1; leds[6] = 1; break;
        case '7': leds[0] = 1; leds[1] = 0; leds[2] = 1; leds[3] = 0; leds[4] = 0; leds[5] = 1; leds[6] = 0; break;
        case '8': leds[0] = 1; leds[1] = 1; leds[2] = 1; leds[3] = 1; leds[4] = 1; leds[5] = 1; leds[6] = 1; break;
        case '9': leds[0] = 1; leds[1] = 1; leds[2] = 1; leds[3] = 1; leds[4] = 0; leds[5] = 1; leds[6] = 1; break;
    }

    cairo_set_line_width(cr, lcd->line_thickness);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    half_thick = ((double) lcd->line_thickness) / 2;
    line_width = lcd->char_width - (lcd->line_thickness * 2) - (half_thick * 2);
    line_height = (lcd->char_height - (lcd->line_thickness * 3) - (half_thick * 4)) / 2;

    for (i = 0; i < 7; i++) {
        x = offset;
        y = lcd->padding + lcd->space_width;

        if (leds[i]) {
            switch (i) {
                case 0:
                    x += lcd->line_thickness + half_thick;
                    y += half_thick;
                    len = line_width;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x + len, y);
                    cairo_stroke(cr);
                    break;
                case 1:
                    x += half_thick;
                    y += lcd->line_thickness + half_thick;
                    len = line_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y + len);
                    cairo_stroke(cr);
                    break;
                case 2:
                    x += lcd->char_width - half_thick;
                    y += lcd->line_thickness + half_thick;
                    len = line_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y + len);
                    cairo_stroke(cr);
                    break;
                case 3:
                    x += lcd->line_thickness + half_thick;
                    y += line_height + lcd->line_thickness + (half_thick * 2) + half_thick;
                    len = line_width;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x + len, y);
                    cairo_stroke(cr);
                    break;
                case 4:
                    x += half_thick;
                    y += line_height + (lcd->line_thickness * 2) + (half_thick * 2) + half_thick;
                    len = line_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y + len);
                    cairo_stroke(cr);
                    break;
                case 5:
                    x += lcd->char_width - half_thick;
                    y += line_height + (lcd->line_thickness * 2) + (half_thick * 2) + half_thick;
                    len = line_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y + len);
                    cairo_stroke(cr);
                    break;
                case 6:
                    x += lcd->line_thickness + half_thick;
                    y += lcd->char_height - half_thick;
                    len = line_width;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x + len, y);
                    cairo_stroke(cr);
                    break;
            }
        }
    }
}

static void paint_separator(GtkLCD *lcd, cairo_t *cr, gint offset, gchar ch)
{
    double x, y, len;
    double half_thick;
    double dist;

    half_thick = ((double) lcd->dot_width) / 2;
    dist = ((double) lcd->char_height) / 6;

    cairo_set_line_width(cr, lcd->dot_width);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    x = offset + half_thick;
    y = lcd->padding + lcd->space_width;

    switch (ch) {
        case ':':
            y += ((double) lcd->char_height) / 2;

            len = lcd->dot_width;
            y -= (dist / 2) + half_thick + len;

            cairo_move_to(cr, x, y);
            cairo_line_to(cr, x, y + len);
            cairo_stroke(cr);

            y += (dist / 2) + half_thick + len;
            y += (dist / 2) + half_thick;

            cairo_move_to(cr, x, y);
            cairo_line_to(cr, x, y + len);
            cairo_stroke(cr);
            break;
        case '.':
            y += lcd->char_height - half_thick;

            cairo_move_to(cr, x, y);
            cairo_line_to(cr, x, y);
            cairo_stroke(cr);
            break;
    }
}

static void gtk_lcd_paint(GtkWidget *widget)
{
    GtkLCD *lcd;
    cairo_t *cr;
    gint offset;
    gint i;

    if (!gtk_widget_get_visible(widget) && !gtk_widget_get_mapped(widget)) {
        return;
    }

    lcd = GTK_LCD(widget);

    cr = gdk_cairo_create(widget->window);

    cairo_set_source_rgb(cr, lcd->bg.red, lcd->bg.green, lcd->bg.blue);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, lcd->fg.red, lcd->fg.green, lcd->fg.blue);

    offset = lcd->padding + lcd->space_width;

    for (i = 0; lcd->format[i]; i++) {
        switch (lcd->format[i]) {
            case '-':
                paint_sign(lcd, cr, offset, lcd->value[i]);
                offset += lcd->sign_width + lcd->space_width;
                break;
            case ' ':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                paint_digit(lcd, cr, offset, lcd->value[i]);
                offset += lcd->char_width + lcd->space_width;
                break;
            case '.': case ':':
                paint_separator(lcd, cr, offset, lcd->value[i]);
                offset += lcd->dot_width + lcd->space_width;
                break;
        }
    }

    cairo_destroy(cr);
}

static void gtk_lcd_destroy(GtkObject *object)
{
    GtkLCD *lcd;
    GtkLCDClass *klass;

    g_return_if_fail(object != NULL);
    g_return_if_fail(GTK_IS_LCD(object));

    lcd = GTK_LCD(object);

    g_free(lcd->format);
    lcd->format = NULL;
    g_free(lcd->value);
    lcd->value = NULL;

    klass = gtk_type_class(gtk_widget_get_type());

    if (GTK_OBJECT_CLASS(klass)->destroy) {
        (*GTK_OBJECT_CLASS(klass)->destroy) (object);
    }
}

