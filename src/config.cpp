/*
 * config.cpp
 *
 *  Created on: 22 ene. 2019
 *      Author: asolo
 */

#include "config.h"

int write_conf_file(struct sDet_conf det_conf,struct sLvw_conf lvw_conf,const char *path)
{
	int rc;
	xmlTextWriterPtr writer;
    int retvalue = -1;

	LIBXML_TEST_VERSION

	// File open
	writer = xmlNewTextWriterFilename(path, 0);
	if (writer == NULL)
		goto cleanup;

	// XML encoding
	rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
	if (rc < 0)
		goto cleanup;

	// Comment
	rc = xmlTextWriterWriteComment(writer, BAD_CAST "Config file");
	if (rc < 0)
		goto cleanup;

	// Element
	rc = xmlTextWriterStartElement(writer, BAD_CAST "config");
	if (rc < 0)
		goto cleanup;

	// Element child
	rc = xmlTextWriterStartElement(writer, BAD_CAST "detection");
	if (rc < 0)
		goto cleanup;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "active", "%d", det_conf.is_active);
	if (rc < 0)
		goto cleanup;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "threshold", "%d", det_conf.threshold);
	if (rc < 0)
		goto cleanup;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "tolerance", "%d", det_conf.tolerance);
	if (rc < 0)
		goto cleanup;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "det_num_shots", "%d", det_conf.det_num_shots);
	if (rc < 0)
		goto cleanup;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "frame_interval", "%f", det_conf.frame_interval);
	if (rc < 0)
		goto cleanup;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "curr_det_num", "%d", det_conf.curr_det_num);
	if (rc < 0)
		goto cleanup;

	/*
	// Close all elements
	rc = xmlTextWriterEndDocument(writer);
	if (rc < 0)
		return -1;
	*/

	// Close the element named "detection"
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0)
		goto cleanup;

	// Element child
	rc = xmlTextWriterStartElement(writer, BAD_CAST "liveview");
	if (rc < 0)
		goto cleanup;

	// Element with value
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "active", "%d", lvw_conf.is_active);
	if (rc < 0)
		goto cleanup;

	// Close the element named "liveview"
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0)
		goto cleanup;

	// Close the element named "config"
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0)
		goto cleanup;


    retvalue = 0;
cleanup:
	xmlFreeTextWriter(writer);
    //Cleanup function for the XML library
    xmlCleanupParser();
    //this is to debug memory for regression tests
    xmlMemoryDump();
	return retvalue;
}

int parse_conf_file(struct sDet_conf *det_conf,struct sLvw_conf *lvw_conf,const char *path)
{
    xmlDoc *doc = NULL;
    xmlNode *root_node = NULL;
    xmlNode *child_node = NULL;
    xmlNode *detection_node = NULL;
    xmlNode *detection_child_node = NULL;
    xmlNode *liveview_node = NULL;
    xmlNode *liveview_child_node = NULL;

    std::string value;

    bool parse_det_elements_check[NUM_DET_PARAMETERS] = {false}; // Boolean array to check if all config value are parsed
    bool parse_lvw_elements_check[NUM_LVW_PARAMETERS] = {false}; // Boolean array to check if all config value are parsed

    int retvalue = -1;
	LIBXML_TEST_VERSION

	doc = xmlParseFile(path);
	if (doc == NULL)
		goto cleanup;

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

    	if(!strncmp((const char *)child_node->name,"liveview",strlen("liveview")))
    	{
    		liveview_node = child_node;
    	}

    }

    // Parse detection config
    if(!detection_node)
    	goto cleanup;

    for (detection_child_node = detection_node->children; detection_child_node; detection_child_node = detection_child_node->next)
    {
    	if(!strncmp((const char *)detection_child_node->name,"active",strlen("active")))
		{
    		value = (char *) xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			det_conf->is_active = std::stoi(value,nullptr);
			parse_det_elements_check[0] = true;
		}
    	else if(!strncmp((const char *)detection_child_node->name,"threshold",strlen("threshold")))
		{
			value = (char *) xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			det_conf->threshold = std::stoi(value,nullptr);
			parse_det_elements_check[1] = true;
		}
    	else if(!strncmp((const char *)detection_child_node->name,"tolerance",strlen("tolerance")))
		{
			value = (char *) xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			det_conf->tolerance = std::stoi(value,nullptr);
			parse_det_elements_check[2] = true;
		}
    	else if(!strncmp((const char *)detection_child_node->name,"det_num_shots",strlen("det_num_shots")))
		{
			value = (char *) xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			det_conf->det_num_shots = std::stoi(value,nullptr);
			parse_det_elements_check[3] = true;
		}
    	else if(!strncmp((const char *)detection_child_node->name,"frame_interval",strlen("frame_interval")))
		{
			value = (char *) xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			det_conf->frame_interval = std::stof(value,nullptr);
			parse_det_elements_check[4] = true;
		}
    	else if(!strncmp((const char *)detection_child_node->name,"curr_det_num",strlen("curr_det_num")))
		{
			value = (char *) xmlNodeListGetString(doc, detection_child_node->xmlChildrenNode, 1);
			det_conf->curr_det_num = std::stoi(value,nullptr);
			parse_det_elements_check[5] = true;
		}
    }

    // Parse detection config
    if(!liveview_node)
    	goto cleanup;

    for (liveview_child_node = liveview_node->children; liveview_child_node; liveview_child_node = liveview_child_node->next)
    {
    	if(!strncmp((const char *)liveview_child_node->name,"active",strlen("active")))
		{
    		value = (char *) xmlNodeListGetString(doc, liveview_child_node->xmlChildrenNode, 1);
    		lvw_conf->is_active = std::stoi(value,nullptr);
			parse_lvw_elements_check[0] = true;
		}
    }


    // Validate XML det parse values
    for(int i = 0; i<NUM_DET_PARAMETERS;i++)
    {
    	if(!parse_det_elements_check[i])
        	goto cleanup;
    }
    // Validate XML lvw parse values
    for(int i = 0; i<NUM_LVW_PARAMETERS;i++)
    {
    	if(!parse_lvw_elements_check[i])
        	goto cleanup;
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
