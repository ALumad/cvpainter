/**
 * SECTION:element-cvpainter
 *
 * FIXME:Describe cvpainter here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! cvpainter ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstcvpainter.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>

GST_DEBUG_CATEGORY_STATIC (gst_cvpainter_debug);
#define GST_CAT_DEFAULT gst_cvpainter_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_cvpainter_parent_class parent_class
G_DEFINE_TYPE (Gstcvpainter, gst_cvpainter, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (cvpainter, "cvpainter", GST_RANK_NONE,
    GST_TYPE_CVPAINTER);

static void gst_cvpainter_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_cvpainter_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_cvpainter_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_cvpainter_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);

/* GObject vmethod implementations */

/* initialize the cvpainter's class */
static void
gst_cvpainter_class_init (GstcvpainterClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_cvpainter_set_property;
  gobject_class->get_property = gst_cvpainter_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple (gstelement_class,
      "cvpainter",
      "FIXME:Generic",
      "FIXME:Generic Template Element", "aleks <<user@hostname.org>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_cvpainter_init (Gstcvpainter * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_cvpainter_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_cvpainter_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  filter->silent = FALSE;
}

static void
gst_cvpainter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstcvpainter *filter = GST_CVPAINTER (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_cvpainter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstcvpainter *filter = GST_CVPAINTER (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_cvpainter_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
	gboolean ret;
	Gstcvpainter *filter;

	filter = GST_CVPAINTER (parent);

	switch (GST_EVENT_TYPE (event)) {
	case GST_EVENT_CAPS:
		{
			GstCaps * caps;

			gst_event_parse_caps (event, &caps);

			GstStructure *structure = gst_caps_get_structure (caps, 0);

			gst_structure_get_int (structure, "width", &filter->width);
			gst_structure_get_int (structure, "height", &filter->height);

			filter->format = gst_structure_get_string (structure, "format");

			//gst_video_info_from_caps (&filter->input_vi, caps);

			ret = gst_pad_event_default (pad, parent, event);
			break;
		}
	default:
		ret = gst_pad_event_default (pad, parent, event);
		break;
	}
	return ret;
}


static GstBuffer *
gst_painter_process_data (Gstcvpainter * filter, GstBuffer * buf)
{
	static int i = 0;
  if (i >100) i = 0;
  GstMapInfo srcmapinfo;
	gst_buffer_map (buf, &srcmapinfo, GST_MAP_READWRITE);

	IplImage * image = cvCreateImageHeader (cvSize (filter->width, filter->height), IPL_DEPTH_8U, 3);
	image->imageData = (char*)srcmapinfo.data;

	cvReleaseImageHeader(&image);
	GstBuffer * outbuf = gst_buffer_new ();
	
	IplImage * dst = cvCreateImageHeader (cvSize (filter->width, filter->height), IPL_DEPTH_8U, 4);
	GstMemory * memory = gst_allocator_alloc (NULL, dst->imageSize, NULL);
	GstMapInfo dstmapinfo;

	if (gst_memory_map(memory, &dstmapinfo, GST_MAP_WRITE)) {
		memcpy (dstmapinfo.data, srcmapinfo.data, srcmapinfo.size);
		dst->imageData = (char*)dstmapinfo.data;
		cvRectangle (dst, cvPoint(10,10), cvPoint(100+i, 100+i), cvScalar(CV_RGB(0, 255, 0)), 2, 0);

		gst_buffer_insert_memory (outbuf, -1, memory);
		gst_memory_unmap(memory, &dstmapinfo);
	}

	cvReleaseImageHeader(&dst);
	
	gst_buffer_unmap(buf, &srcmapinfo);
  //i++;
  return outbuf;  
}



/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_cvpainter_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
	Gstcvpainter *filter;

	filter = GST_CVPAINTER (parent);

	if (filter->silent == FALSE)
		g_print ("I'm plugged, therefore I'm in.\n");


	GstBuffer *outbuf;
	outbuf = gst_painter_process_data (filter, buf);

	gst_buffer_unref (buf);
	if (!outbuf) {
		/* something went wrong - signal an error */
		GST_ELEMENT_ERROR (GST_ELEMENT (filter), STREAM, FAILED, (NULL), (NULL));
		return GST_FLOW_ERROR;
	}

	return gst_pad_push (filter->srcpad, outbuf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
cvpainter_init (GstPlugin * cvpainter)
{
  /* debug category for filtering log messages
   *
   * exchange the string 'Template cvpainter' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_cvpainter_debug, "cvpainter",
      0, "Template cvpainter");

  return GST_ELEMENT_REGISTER (cvpainter, cvpainter);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "gstcvpainter"
#endif


#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "1.0.0"
#endif



#ifndef GST_LICENSE
#define GST_LICENSE "LGPL"
#endif


#ifndef GST_PACKAGE_NAME
#define GST_PACKAGE_NAME "cvpainter"
#endif

#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "cvpainter"
#endif




GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    cvpainter,
    "cvpainter",
    cvpainter_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
