#include "genicam.h"
#include "debug.h"

#include "imgui.h"

#include <assert.h>

Genicam::Genicam() {
    device = NULL;
    genicam = NULL;
//    update = false;
}

Genicam::~Genicam() {
    device = NULL;
    genicam = NULL;
//    update = false;
}

void Genicam::initialize(ArvCamera *_camera) {
    assert(_camera != NULL);
    device = arv_camera_get_device(_camera);
    assert(device != NULL);
    genicam = arv_device_get_genicam(device);
    assert(genicam != NULL);
//    update = true;
}

void Genicam::destroy(void) {
    device = NULL;
    genicam = NULL;
//    update = false;
}

void Genicam::listFeatures(void) {
//    if (update) {
//        gint64 start = g_get_monotonic_time();
//        showFeature("Root", 0);
        traverse("Root");
//        D("Executed in %g s\n", (g_get_monotonic_time() - start) / 1000000.0);
//        update = false;
//    }
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

void Genicam::traverse(const char *_name) {
	ArvGcNode *node;

    node = arv_gc_get_node(genicam, _name);
	if (ARV_IS_GC_FEATURE_NODE(node) && arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE(node), NULL)) {
		if (ARV_IS_GC_CATEGORY(node)) {
            // skip the root node, categories are handled below
            if (strncmp(_name, "Root", 4) != 0) {
                handleCategory(_name);
            }

            // recurse into the category..
            const GSList *features = arv_gc_category_get_features(ARV_GC_CATEGORY(node));
			const GSList *iter;
            for (iter = features; iter != NULL; iter = iter->next) {
                traverse((const char *)(iter->data));
            }
//		} else {
//            if (arv_gc_feature_node_is_available(ARV_GC_FEATURE_NODE(node), NULL)) {
//                printf("%s: '%s'\n", arv_dom_node_get_node_name(ARV_DOM_NODE(node)), _name);
//            }
        }
    }
}

void Genicam::handleCategory(const char *_name) {
    assert(_name != NULL);
    ArvGcNode *node = arv_gc_get_node(genicam, _name);
    assert(node != NULL);

    //printf("%s: name '%s'\n", arv_dom_node_get_node_name(ARV_DOM_NODE(node)), _name);

    ImGui::AlignTextToFramePadding();
    bool isOpen = ImGui::TreeNode(_name);
    ImGui::NextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", "");
    ImGui::NextColumn();
    if (isOpen) {
        const GSList *features;
        const GSList *iter;
        features = arv_gc_category_get_features(ARV_GC_CATEGORY(node));
        // handle individual features
        for (iter = features; iter != NULL; iter = iter->next) {
            handleFeature((const char *)(iter->data));
        }
        ImGui::TreePop();
    }

}

void Genicam::handleFeature(const char *_name) {
    assert(_name != NULL);
    ArvGcNode *node = arv_gc_get_node(genicam, _name);
    assert(node != NULL);

    assert(ARV_IS_GC_CATEGORY(node) != true);
    if (ARV_IS_GC_CATEGORY(node)) {
        E("skipping category node %s\n", _name);
        return;
    }

    if (arv_gc_feature_node_is_available(ARV_GC_FEATURE_NODE(node), NULL)) {
        //printf("%s: '%s'\n", arv_dom_node_get_node_name(ARV_DOM_NODE(node)), _name);
        ImGui::AlignTextToFramePadding();
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
        ImGui::TreeNodeEx("Field", flags, "%s", _name);
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);

        if (ARV_IS_GC_STRING(node) || ARV_IS_GC_ENUMERATION(node)) {
            handleFeatureString(node);
        } else if (ARV_IS_GC_INTEGER(node)) {
            handleFeatureInteger(node);
        } else if (ARV_IS_GC_FLOAT(node)) {
            handleFeatureFloat(node);
        } else if (ARV_IS_GC_BOOLEAN(node)) {
            handleFeatureBoolean(node);
        } else if (ARV_IS_GC_COMMAND(node)) {
            handleFeatureCommand(node);
        } else {
            ImGui::Text("TODO TODO TODO");
        }
        ImGui::NextColumn();
    }
}

void Genicam::handleFeatureInteger(ArvGcNode *_node) {
    const char *unit = arv_gc_integer_get_unit(ARV_GC_INTEGER(_node));
    GError *error = NULL;
    if (ARV_IS_GC_ENUMERATION(_node)) {
        const char *value = arv_gc_string_get_value(ARV_GC_STRING(_node), &error);
        assert(error == NULL);
        ImGui::Text("%s", value);
    } else {
        gint64 value = arv_gc_integer_get_value(ARV_GC_INTEGER(_node), &error);
        assert(error == NULL);
        ImGui::Text("%ld %s", value, unit != NULL ? unit : "");
    }
}

void Genicam::handleFeatureFloat(ArvGcNode *_node) {
    const char *unit = arv_gc_float_get_unit(ARV_GC_FLOAT(_node));
    GError *error = NULL;
    double value = arv_gc_float_get_value(ARV_GC_FLOAT(_node), &error);
    assert(error == NULL);
    ImGui::Text("%.3f %s", value, unit != NULL ? unit : "");
}

void Genicam::handleFeatureBoolean(ArvGcNode *_node) {
    GError *error = NULL;
    bool value = arv_gc_boolean_get_value(ARV_GC_BOOLEAN(_node), &error);
    assert(error == NULL);
    ImGui::Text("%s", value ? "true" : "false");
}

void Genicam::handleFeatureString(ArvGcNode *_node) {
    GError *error = NULL;
    const char *value = arv_gc_string_get_value(ARV_GC_STRING(_node), &error);
    assert(error == NULL);
    ImGui::Text("%s", value);
}

void Genicam::handleFeatureCommand(ArvGcNode *_node) {
    ImGui::Text("%s", "COMMAND");
}
