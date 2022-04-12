#include <string> 

void GenerateInputFile (char *filename)
{
	TiXmlDocument doc;  
	TiXmlElement* msg;
 	TiXmlElement * XMLSystem = new TiXmlElement("system");  
	doc.LinkEndChild(XMLSystem);  
	XMLSystem->SetAttribute("modnum", );
	XMLSystem->SetAttribute("tasknum", );
	XMLSystem->SetAttribute("bandwidth", );
	XMLSystem->SetAttribute("mesnum", );
	
	TiXmlElement * modules = new TiXmlElement("modules");
	XMLSystem->LinkEndChild(modules);
	TiXmlElement * pc = new TiXmlElement("pc");	
	for (size_t i = 1; i < ModNum; i++)
	{
		pc->LinkEndChild(new TiXmlText(std::to_string(PCVector[i])));
		modules->LinkEndChild(pc);
		pc = new TiXmlElement("pc")
	}

	TiXmlElement * tasks = new TiXmlElement("tasks");
	XMLSystem->LinkEndChild(tasks);
	TiXmlElement * task = new TiXmlElement("task");	
	for (size_t i = 1; i < TaskNum; i++)
	{
		task->SetAttribute("period", );
		task->SetAttribute("time", );
		task->SetAttribute("left", );
		task->SetAttribute("right", );
		task->SetAttribute("messize", );
		tasks->LinkEndChild(task);
		task = new TiXmlElement("task")
	}

	TiXmlElement * messages = new TiXmlElement("messages");
	XMLSystem->LinkEndChild(messages);
	TiXmlElement * mes = new TiXmlElement("mes");	
	for (size_t i = 1; i < MesNum; i++)
	{
		mes->SetAttribute("src", );
		mes->SetAttribute("dest", );
		mes->SetAttribute("size", );
		messages->LinkEndChild(mes);
		mes = new TiXmlElement("mes")
	}
	doc.SaveFile(filename);
	return;  
} 