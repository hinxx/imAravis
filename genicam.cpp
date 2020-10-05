#include "genicam.h"
#include "debug.h"

#include <assert.h>

Genicam::Genicam() {
    device = NULL;
    genicam = NULL;
}

Genicam::~Genicam() {
    device = NULL;
    genicam = NULL;
}

void Genicam::initialize(ArvCamera *_camera) {
    assert(_camera != NULL);
    device = arv_camera_get_device(_camera);
    assert(device != NULL);
    genicam = arv_device_get_genicam(device);
    assert(genicam != NULL);
}


void Genicam::listFeatures(void) {
//    genicam, "Root", ARV_TOOL_LIST_MODE_FEATURES, 0
    showFeature("Root", 0);
}

void Genicam::showFeature(const char *feature, int level) {
	ArvGcNode *node;

	node = arv_gc_get_node(genicam, feature);
	if (ARV_IS_GC_FEATURE_NODE(node) && arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE(node), NULL)) {

		if (ARV_IS_GC_CATEGORY(node)) {
			printf("%*s%-12s: '%s'\n", 4 * level, "", arv_dom_node_get_node_name(ARV_DOM_NODE(node)), feature);
		} else {
			if (arv_gc_feature_node_is_available(ARV_GC_FEATURE_NODE(node), NULL)) {
				char *value = NULL;
				GError *error = NULL;
				gboolean is_selector;
//				if (list_mode == ARV_TOOL_LIST_MODE_VALUES) {
					const char *unit;

					if (ARV_IS_GC_STRING(node) || ARV_IS_GC_ENUMERATION(node)) {
						value = g_strdup_printf ("'%s'", arv_gc_string_get_value (ARV_GC_STRING (node), &error));
					} else if (ARV_IS_GC_INTEGER(node)) {
						if (ARV_IS_GC_ENUMERATION(node)) {
							value = g_strdup_printf("'%s'", arv_gc_string_get_value(ARV_GC_STRING(node), &error));
						} else {
							unit = arv_gc_integer_get_unit(ARV_GC_INTEGER(node));
							value = g_strdup_printf("%" G_GINT64_FORMAT "%s%s",
										 arv_gc_integer_get_value(ARV_GC_INTEGER(node), &error),
										 unit != NULL ? " " : "",
										 unit != NULL ? unit : "");
						}
					} else if (ARV_IS_GC_FLOAT(node)) {
						unit = arv_gc_float_get_unit(ARV_GC_FLOAT(node));
						value = g_strdup_printf("%g%s%s",
									 arv_gc_float_get_value(ARV_GC_FLOAT(node), &error),
									 unit != NULL ? " " : "",
									 unit != NULL ? unit : "");
					} else if (ARV_IS_GC_BOOLEAN(node)) {
						value = g_strdup_printf("%s",
									 arv_gc_boolean_get_value(ARV_GC_BOOLEAN(node), &error) ?
									 "true" : "false");
					}
//				}

				is_selector = ARV_IS_GC_SELECTOR(node) && arv_gc_selector_is_selector(ARV_GC_SELECTOR(node));

				if (error != NULL) {
					g_clear_error(&error);
				} else {
					if (value != NULL && value[0] != '\0')
						printf("%*s%-12s: '%s' = %s\n", 4 * level, "",
							arv_dom_node_get_node_name(ARV_DOM_NODE(node)), feature, value);
					else
						printf("%*s%-12s: '%s'\n", 4 * level, "",
							arv_dom_node_get_node_name(ARV_DOM_NODE (node)), feature);

					if (is_selector) {
						const GSList *iter;
						for (iter = arv_gc_selector_get_selected_features(ARV_GC_SELECTOR(node)); iter != NULL; iter = iter->next) {
							printf(" %*s     * %s\n", 4 * level, " ", arv_gc_feature_node_get_name((ArvGcFeatureNode *)(iter->data)));
						}
					}
				}
				g_clear_pointer(&value, g_free);
			} else {
//				if (list_mode == ARV_TOOL_LIST_MODE_FEATURES)
					printf("%*s%-12s: '%s' (Not available)\n", 4 * level, "", arv_dom_node_get_node_name(ARV_DOM_NODE(node)), feature);
			}
		}

//		if (list_mode == ARV_TOOL_LIST_MODE_DESCRIPTIONS) {
			const char *description;
			description = arv_gc_feature_node_get_description(ARV_GC_FEATURE_NODE(node));
			if (description)
				printf("%s\n", description);
//		}

		if (ARV_IS_GC_CATEGORY(node)) {
			const GSList *features;
			const GSList *iter;
			features = arv_gc_category_get_features(ARV_GC_CATEGORY(node));
            for (iter = features; iter != NULL; iter = iter->next) {
//                arv_tool_list_features(genicam, iter->data, list_mode, level + 1);
                showFeature((const char *)(iter->data), level + 1);
            }

//		} else if (ARV_IS_GC_ENUMERATION (node) && list_mode == ARV_TOOL_LIST_MODE_FEATURES) {
		} else if (ARV_IS_GC_ENUMERATION(node)) {
			const GSList *childs;
			const GSList *iter;
			childs = arv_gc_enumeration_get_entries(ARV_GC_ENUMERATION(node));
			for (iter = childs; iter != NULL; iter = iter->next) {
				if (arv_gc_feature_node_is_implemented((ArvGcFeatureNode *)(iter->data), NULL)) {
					printf("%*s%-12s: '%s'%s\n", 4 * (level + 1), "",
						arv_dom_node_get_node_name((ArvDomNode *)iter->data),
						arv_gc_feature_node_get_name((ArvGcFeatureNode *)(iter->data)),
						arv_gc_feature_node_is_available((ArvGcFeatureNode *)(iter->data), NULL) ? "" : " (Not available)");
				}
			}
		}
	}
}
