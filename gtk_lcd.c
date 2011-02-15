#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "gtk_lcd.h"

struct _GtkLCDPrivate {
    gint length;
    gchar *format;
    gchar *value;

    gint padding;
    gint char_height;
    gint char_width;
    gint dot_width;
    gint sign_width;
    gint space_width;
    gint line_thickness;

    struct {
        double red, green, blue;
    } fg, bg;
};

typedef struct _GtkLCDPrivate GtkLCDPrivate;

static void gtk_lcd_class_init(GtkLCDClass *klass);
static void gtk_lcd_init(GtkLCD *lcd);
static void gtk_lcd_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void gtk_lcd_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void gtk_lcd_realize(GtkWidget *widget);
static gboolean gtk_lcd_expose(GtkWidget *widget, GdkEventExpose *event);
static void gtk_lcd_paint(GtkWidget *widget);
static void gtk_lcd_finalize(GObject *object);

G_DEFINE_TYPE(GtkLCD, gtk_lcd, GTK_TYPE_WIDGET)

static void gtk_lcd_class_init(GtkLCDClass *klass)
{
    GtkWidgetClass *widget_class;
    GtkObjectClass *object_class;
    GObjectClass *gobject_class;

    widget_class = (GtkWidgetClass *) klass;
    object_class = (GtkObjectClass *) klass;
    gobject_class = G_OBJECT_CLASS(klass);

    widget_class->realize = gtk_lcd_realize;
    widget_class->size_request = gtk_lcd_size_request;
    widget_class->size_allocate = gtk_lcd_size_allocate;
    widget_class->expose_event = gtk_lcd_expose;

    gobject_class->finalize = gtk_lcd_finalize;

    g_type_class_add_private(klass, sizeof(GtkLCDPrivate));
}

static void gtk_lcd_init(GtkLCD *lcd)
{
    GtkLCDPrivate *priv;

    priv = G_TYPE_INSTANCE_GET_PRIVATE(lcd, gtk_lcd_get_type(), GtkLCDPrivate);
    lcd->priv = priv;

    priv->length = 0;
    priv->format = NULL;
    priv->value = NULL;
    
    priv->padding = 2;
    priv->char_height = 27;
    priv->char_width = 12;
    priv->dot_width = 3;
    priv->sign_width = 6;
    priv->space_width = 2;
    priv->line_thickness = 3;

    priv->fg.red = 0.6;
    priv->fg.green = 1.0;
    priv->fg.blue = 0.0;

    priv->bg.red = 0.0;
    priv->bg.green = 0.0;
    priv->bg.blue = 0.0;
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

void gtk_lcd_set_format(GtkLCD *lcd, const gchar *format)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    g_free(priv->format);
    g_free(priv->value);
    priv->format = g_strdup(format);
    priv->length = strlen(format);
    priv->value = g_strnfill(priv->length, ' ');
    gtk_widget_queue_resize(GTK_WIDGET(lcd));
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_value(GtkLCD *lcd, const gchar *value)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    g_strlcpy(priv->value, value, priv->length + 1);
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_padding(GtkLCD *lcd, gint val)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    priv->padding = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_char_height(GtkLCD *lcd, gint val)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    priv->char_height = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_char_width(GtkLCD *lcd, gint val)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    priv->char_width = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_dot_width(GtkLCD *lcd, gint val)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    priv->dot_width = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_sign_width(GtkLCD *lcd, gint val)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    priv->sign_width = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_space_width(GtkLCD *lcd, gint val)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    priv->space_width = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_line_thickness(GtkLCD *lcd, gint val)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    priv->line_thickness = val;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_fg(GtkLCD *lcd, double red, double green, double blue)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    priv->fg.red = red;
    priv->fg.green = green;
    priv->fg.blue = blue;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

void gtk_lcd_set_bg(GtkLCD *lcd, double red, double green, double blue)
{
    GtkLCDPrivate *priv;

    priv = (GtkLCDPrivate *) lcd->priv;
    priv->bg.red = red;
    priv->bg.green = green;
    priv->bg.blue = blue;
    gtk_widget_queue_draw(GTK_WIDGET(lcd));
}

static void gtk_lcd_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    GtkLCD *lcd;
    GtkLCDPrivate *priv;
    gint width;
    gint height;
    gchar *p;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_LCD(widget));
    g_return_if_fail(requisition != NULL);

    lcd = GTK_LCD(widget);
    priv = (GtkLCDPrivate *) lcd->priv;

    width = priv->padding * 2 + priv->space_width;
    height = priv->padding * 2 + priv->char_height + priv->space_width * 2;

    for (p = priv->format; *p; p++) {
        switch (*p) {
            case '-':
            case '+':
                width += priv->sign_width;
                break;
            case '0':
            case ' ':
                width += priv->char_width;
                break;
            case '.':
            case ':':
                width += priv->dot_width;
                break;
        }
        width += priv->space_width;
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
    GtkLCDPrivate *priv;
    double x, y, len;

    priv = (GtkLCDPrivate *) lcd->priv;

    switch (ch) {
        case '-':
            y = priv->padding + priv->space_width;

            x = offset;
            x += ((double) priv->line_thickness) / 2;
            y += ((double) priv->char_height) / 2;
            len = priv->sign_width - priv->line_thickness;
            cairo_move_to(cr, x, y);
            cairo_line_to(cr, x + len, y);
            cairo_set_line_width(cr, priv->line_thickness);
            cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
            cairo_stroke(cr);
            break;
    }
}

static void paint_digit(GtkLCD *lcd, cairo_t *cr, gint offset, gchar ch)
{
    GtkLCDPrivate *priv;
    gint leds[7];
    gint i;
    double half_thick;
    gint line_width;
    gint line_height;
    double x, y, len;

    priv = (GtkLCDPrivate *) lcd->priv;

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

    cairo_set_line_width(cr, priv->line_thickness);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    half_thick = ((double) priv->line_thickness) / 2;
    line_width = priv->char_width - (priv->line_thickness * 2) - (half_thick * 2);
    line_height = (priv->char_height - (priv->line_thickness * 3) - (half_thick * 4)) / 2;

    for (i = 0; i < 7; i++) {
        x = offset;
        y = priv->padding + priv->space_width;

        if (leds[i]) {
            switch (i) {
                case 0:
                    x += priv->line_thickness + half_thick;
                    y += half_thick;
                    len = line_width;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x + len, y);
                    cairo_stroke(cr);
                    break;
                case 1:
                    x += half_thick;
                    y += priv->line_thickness + half_thick;
                    len = line_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y + len);
                    cairo_stroke(cr);
                    break;
                case 2:
                    x += priv->char_width - half_thick;
                    y += priv->line_thickness + half_thick;
                    len = line_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y + len);
                    cairo_stroke(cr);
                    break;
                case 3:
                    x += priv->line_thickness + half_thick;
                    y += line_height + priv->line_thickness + (half_thick * 2) + half_thick;
                    len = line_width;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x + len, y);
                    cairo_stroke(cr);
                    break;
                case 4:
                    x += half_thick;
                    y += line_height + (priv->line_thickness * 2) + (half_thick * 2) + half_thick;
                    len = line_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y + len);
                    cairo_stroke(cr);
                    break;
                case 5:
                    x += priv->char_width - half_thick;
                    y += line_height + (priv->line_thickness * 2) + (half_thick * 2) + half_thick;
                    len = line_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y + len);
                    cairo_stroke(cr);
                    break;
                case 6:
                    x += priv->line_thickness + half_thick;
                    y += priv->char_height - half_thick;
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
    GtkLCDPrivate *priv;
    double x, y, len;
    double half_thick;
    double dist;

    priv = (GtkLCDPrivate *) lcd->priv;

    half_thick = ((double) priv->dot_width) / 2;
    dist = ((double) priv->char_height) / 6;

    cairo_set_line_width(cr, priv->dot_width);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    x = offset + half_thick;
    y = priv->padding + priv->space_width;

    switch (ch) {
        case ':':
            y += ((double) priv->char_height) / 2;

            len = priv->dot_width;
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
            y += priv->char_height - half_thick;

            cairo_move_to(cr, x, y);
            cairo_line_to(cr, x, y);
            cairo_stroke(cr);
            break;
    }
}

static void gtk_lcd_paint(GtkWidget *widget)
{
    GtkLCD *lcd;
    GtkLCDPrivate *priv;
    cairo_t *cr;
    gint offset;
    gint i;

    if (!gtk_widget_get_visible(widget) && !gtk_widget_get_mapped(widget)) {
        return;
    }

    lcd = GTK_LCD(widget);
    priv = (GtkLCDPrivate *) lcd->priv;

    cr = gdk_cairo_create(widget->window);

    cairo_set_source_rgb(cr, priv->bg.red, priv->bg.green, priv->bg.blue);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, priv->fg.red, priv->fg.green, priv->fg.blue);

    offset = priv->padding + priv->space_width;

    for (i = 0; priv->format[i]; i++) {
        switch (priv->format[i]) {
            case '-':
                paint_sign(lcd, cr, offset, priv->value[i]);
                offset += priv->sign_width + priv->space_width;
                break;
            case ' ':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                paint_digit(lcd, cr, offset, priv->value[i]);
                offset += priv->char_width + priv->space_width;
                break;
            case '.': case ':':
                paint_separator(lcd, cr, offset, priv->value[i]);
                offset += priv->dot_width + priv->space_width;
                break;
        }
    }

    cairo_destroy(cr);
}

static void gtk_lcd_finalize(GObject *object)
{
    GtkLCD *lcd;
    GtkLCDPrivate *priv;
    GtkLCDClass *klass;

    g_return_if_fail(object != NULL);
    g_return_if_fail(GTK_IS_LCD(object));

    lcd = GTK_LCD(object);
    priv = (GtkLCDPrivate *) lcd->priv;

    g_free(priv->format);
    g_free(priv->value);

    G_OBJECT_CLASS(gtk_lcd_parent_class)->finalize(object);
}

