/*
 * config.cpp
 *
 *  Created on: 22 ene. 2019
 *      Author: asolo
 */

#include "config.h"


int write_conf_file(struct sDet_conf det_conf, const char *path)
{
	int rc;
	xmlTextWriterPtr writer;
	xmlChar *tmp;

	LIBXML_TEST_VERSION

	// File open
	writer = xmlNewTextWriterFilename(path, 0);
	if (writer == NULL)
		return -1;

	// XML encoding
	rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
	if (rc < 0)
		return -1;

	// Comment
	rc = xmlTextWriterWriteComment(writer, BAD_CAST "Config file");
	if (rc < 0)
		return -1;

	// Element
	rc = xmlTextWriterStartElement(writer, BAD_CAST "config");
	if (rc < 0)
		return -1;

	// Element child
	rc = xmlTextWriterStartElement(writer, BAD_CAST "detection");
	if (rc < 0)
		return -1;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "active", "%d", det_conf.is_active);
	if (rc < 0)
		return -1;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "threshold", "%d", det_conf.threshold);
	if (rc < 0)
		return -1;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "tolerance", "%d", det_conf.tolerance);
	if (rc < 0)
		return -1;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "num_frames", "%d", det_conf.num_frames);
	if (rc < 0)
		return -1;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "curr_det_num", "%d", det_conf.curr_det_num);
	if (rc < 0)
		return -1;

	/*
	// Close all elements
	rc = xmlTextWriterEndDocument(writer);
	if (rc < 0)
		return -1;
	*/

	// Close the element named "detection"
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0)
		return -1;

	// Close the element named "config"
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0)
		return -1;

	xmlFreeTextWriter(writer);

    //Cleanup function for the XML library
    xmlCleanupParser();

    //this is to debug memory for regression tests
    xmlMemoryDump();
	return 0;
}

int parse_conf_file(struct sDet_conf *det_conf, const char *path)
{
    xmlDoc *doc = NULL;
    xmlNode *root_node = NULL;
    xmlNode *child_node = NULL;
    xmlNode *detection_node = NULL;
    xmlNode *detection_child_node = NULL;
    xmlChar *value;

    int retvalue = -1;
	LIBXML_TEST_VERSION

	doc = xmlParseFile(path);
	if (doc == NULL)
			return -1;

    // Get the root node
	root_node = xmlDocGetRootElement(doc);

    // Check root node
    if(strncmp((const char *)root_node->name,"config",strlen("config")))
    	goto cleanup;

    // Check child nodes
    for (child_node = root_node->children; child_node; child_node = child_node->next)
    {
    	if(!strncmp((const char *)child_node->name,"detection",strlen("detection")))
    	{
    		detection_node = child_node;
    	}

    }

    // Parse detection config
    if(!detection_node)
    	goto cleanup;

    for (detection_child_node = detection_node->children; detection_child_node; detection_child_node = detection_child_node->next)
    {
    	if(!strncmp((const char *)detection_child_node->name,"active",strlen("threshold")))
		{
			value = xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			printf("node type: Element, name: %s, value: %s\n", detection_child_node->name,value);
		}
    	else if(!strncmp((const char *)detection_child_node->name,"threshold",strlen("threshold")))
		{
			value = xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			printf("node type: Element, name: %s, value: %s\n", detection_child_node->name,value);
		}
    	else if(!strncmp((const char *)detection_child_node->name,"tolerance",strlen("threshold")))
		{
			value = xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			printf("node type: Element, name: %s, value: %s\n", detection_child_node->name,value);
		}
    	else if(!strncmp((const char *)detection_child_node->name,"num_frames",strlen("threshold")))
		{
			value = xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			printf("node type: Element, name: %s, value: %s\n", detection_child_node->name,value);
		}
    	else if(!strncmp((const char *)detection_child_node->name,"curr_det_num",strlen("threshold")))
		{
			value = xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			printf("node type: Element, name: %s, value: %s\n", detection_child_node->name,value);
		}
    }



    retvalue = 0;
cleanup:
	xmlFreeDoc(doc);
    //Cleanup function for the XML library
    xmlCleanupParser();
    //this is to debug memory for regression tests
    xmlMemoryDump();
	return retvalue;
}
