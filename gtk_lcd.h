#ifndef __GTK_LCD_H
#define __GTK_LCD_H

G_BEGIN_DECLS

#define GTK_LCD(obj) GTK_CHECK_CAST(obj, gtk_lcd_get_type (), GtkLCD)
#define GTK_LCD_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_lcd_get_type(), GtkLCDClass)
#define GTK_IS_LCD(obj) GTK_CHECK_TYPE(obj, gtk_lcd_get_type())
typedef struct _GtkLCD GtkLCD;
typedef struct _GtkLCDClass GtkLCDClass;


struct _GtkLCD {
    GtkWidget widget;

    gint  length;
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

struct _GtkLCDClass {
    GtkWidgetClass parent_class;
};


GtkType gtk_lcd_get_type(void);
void gtk_lcd_set_format(GtkLCD *lcd, const gchar *format);
void gtk_lcd_set_value(GtkLCD *lcd, const gchar *value);
void gtk_lcd_set_padding(GtkLCD *lcd, gint val);
void gtk_lcd_set_char_height(GtkLCD *lcd, gint val);
void gtk_lcd_set_char_width(GtkLCD *lcd, gint val);
void gtk_lcd_set_dot_width(GtkLCD *lcd, gint val);
void gtk_lcd_set_sign_width(GtkLCD *lcd, gint val);
void gtk_lcd_set_space_width(GtkLCD *lcd, gint val);
void gtk_lcd_set_line_thickness(GtkLCD *lcd, gint val);
void gtk_lcd_set_fg(GtkLCD *lcd, double red, double green, double blue);
void gtk_lcd_set_bg(GtkLCD *lcd, double red, double green, double blue);
GtkWidget *gtk_lcd_new();


G_END_DECLS

#endif

