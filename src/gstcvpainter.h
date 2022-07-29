#ifndef __GST_CVPAINTER_H__
#define __GST_CVPAINTER_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_CVPAINTER (gst_cvpainter_get_type())
G_DECLARE_FINAL_TYPE (Gstcvpainter, gst_cvpainter,
    GST, CVPAINTER, GstElement)

struct _Gstcvpainter
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  gboolean silent;
  
  const char *format;

  int width;
  int height;

};

G_END_DECLS

#endif /* __GST_CVPAINTER_H__ */
