#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "MainClasses.h"
#include "TinyXml/tinyxml.h"
#include "TinyXml/tinystr.h"

void Swap(int* a, int* b) 
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
    return;
}

int PairGCD(int a, int b) 
{
    if (a < b) {
        Swap(&a, &b);
    }
    while (a % b != 0) {
        a = a % b;
        Swap(&a, &b);
    }
    return b;
}

int PairLCM(int a, int b) 
{
    return (a * b) / PairGCD(a, b);
}

int LCM(std::vector<int> Periods) 
{
    if (Periods.size() == 0)
    {
        return -1;
    }
    if (Periods.size() == 1)
    {
        return Periods[0];
    }
    int Result = PairLCM(Periods[0], Periods[1]);
    for (size_t i = 2; i < Periods.size(); i++) 
    {
        Result = PairLCM(Result, Periods[i]);
    }
    return Result;
}

System::System (char * FileName)
{
    int ModNum, PCNum, k = 0;
    int TaskNum, Period, Num;
    double CMessageSize, Time, Left, Right;
    int MessageNum, Src, Dest, Size;
    std::vector<int> PeriosVector;
    
    TiXmlDocument Document(FileName);
    bool IsLoaded = Document.LoadFile();
    
    if (!IsLoaded)
    {
        //std::cout << "Input File didn't loaded" << std::endl;
    }
    
    TiXmlElement* XMLSystem = Document.FirstChildElement("system");
    TiXmlAttribute* pAttrib = XMLSystem->FirstAttribute();
    if (pAttrib->QueryIntValue(&ModNum) != TIXML_SUCCESS) {
        std::cerr << "Input file format error" << std::endl; 
        exit(2);
    }
    pAttrib=pAttrib->Next();
    if (pAttrib->QueryIntValue(&TaskNum) != TIXML_SUCCESS) {
        std::cerr << "Input file format error" << std::endl; 
        exit(2);
    }
    pAttrib=pAttrib->Next();
    if (pAttrib->QueryDoubleValue(&BTotal) != TIXML_SUCCESS) {
        std::cerr << "Input file format error" << std::endl; 
        exit(2);
    }
    pAttrib=pAttrib->Next();
    if (pAttrib->QueryIntValue(&MessageNum) != TIXML_SUCCESS) {
        std::cerr << "Input file format error" << std::endl; 
        exit(2);
    }
    
    XMLSystem = XMLSystem->FirstChildElement("modules");
    TiXmlElement* XMLPC = XMLSystem->FirstChildElement("pc");
    for (int i = 0; i < ModNum; i++)
    {
        std::string InputStr = XMLPC->GetText();
        std::stringstream Str(InputStr);
        Str >> PCNum;
        for (int j=0; j< PCNum; j++)
        {
            SystemPC.push_back(std::make_shared<PC>(i));
            SystemPC[k]->Num = k;
            k++;
        }
        XMLPC = XMLPC->NextSiblingElement("pc");        
    }
    
    XMLSystem = XMLSystem->NextSiblingElement("tasks");
    TiXmlElement* task = XMLSystem->FirstChildElement("task");

    for (int i = 0; i < TaskNum; i++)
    {   
        pAttrib = task->FirstAttribute();
    
        if (pAttrib->QueryIntValue(&Period) != TIXML_SUCCESS) {
            std::cerr << "Input file format error" << std::endl; 
            exit(2);
        }
        pAttrib = pAttrib->Next();
        if (pAttrib->QueryDoubleValue(&Time) != TIXML_SUCCESS) {
            std::cerr << "Input file format error" << std::endl; 
            exit(2);
        }
        pAttrib = pAttrib->Next();
        if (pAttrib->QueryDoubleValue(&Left) != TIXML_SUCCESS) {
            std::cerr << "Input file format error" << std::endl; 
            exit(2);
        }
        pAttrib = pAttrib->Next();
        if (pAttrib->QueryDoubleValue(&Right) != TIXML_SUCCESS) {
            std::cerr << "Input file format error" << std::endl; 
            exit(2);
        }
        pAttrib = pAttrib->Next();
        if (pAttrib->QueryDoubleValue(&CMessageSize) != TIXML_SUCCESS) {
            std::cerr << "Input file format error" << std::endl; 
            exit(2);
        }

        PeriosVector.emplace_back(Period);
        SystemTask.push_back(std::make_shared<Task>(
                             Period, Time, Left, Right, CMessageSize));
        SystemTask[i]->InitLeft = SystemTask[i]->Left;
        SystemTask[i]->MaxLeft = SystemTask[i]->Left;
        SystemTask[i]->MinLeft = SystemTask[i]->Left;
        SystemTask[i]->InitRight = SystemTask[i]->Right;
        task = task->NextSiblingElement("task");
    
    }

    LCMPeriod = LCM(PeriosVector);
    
    XMLSystem = XMLSystem->NextSiblingElement("messages");
    TiXmlElement* mes = XMLSystem->FirstChildElement("mes");
    for (int i = 0; i < MessageNum; i++)
    {   
        pAttrib = mes->FirstAttribute();
    
        if (pAttrib->QueryIntValue(&Src) != TIXML_SUCCESS) {
            std::cerr << "Input file format error" << std::endl; 
            exit(2);
        }
        pAttrib = pAttrib->Next();
        if (pAttrib->QueryIntValue(&Dest) != TIXML_SUCCESS) {
            std::cerr << "Input file format error" << std::endl; 
            exit(2);
        }
        pAttrib = pAttrib->Next();
        if (pAttrib->QueryIntValue(&Size) != TIXML_SUCCESS) {
            std::cerr << "Input file format error" << std::endl; 
            exit(2);
        }
        SystemMessage.push_back(std::make_shared<Message>(
                                SystemTask[Src], SystemTask[Dest], Size));
        SystemTask[Src]->OutMessage.emplace_back(Dest);
        SystemTask[Src]->MesOut.push_back(std::weak_ptr<Message>(SystemMessage[i]));
        SystemTask[Dest]->InMessage.emplace_back(Src);
        SystemTask[Dest]->MesIn.emplace_back(std::weak_ptr<Message>(SystemMessage[i]));
        SystemMessage[i]->DestNum = Dest;
        SystemMessage[i]->SrcNum = Src;
        SystemMessage[i]->StabilityCoef = StableCoef(LCMPeriod / SystemTask[Src]->Period);

        mes = mes->NextSiblingElement("mes");
    
    }
    
    return;

}


void System:: PrintSystem()
{
    //std::cout << "========SYSTEM========" << std::endl; 
    //std::cout << "--------PC--------:" << std::endl;
    for (size_t i = 0; i < SystemPC.size(); i++)
    {
        //std::cout << std::endl;
        //std::cout << "       PC " << i << std::endl;
        //std::cout << "Module: " << SystemPC[i]->ModNum << std::endl;
        //std::cout << "PPoint: " << SystemPC[i]->PC_PPoint << std::endl;
        //std::cout << "Num: " << SystemPC[i]->Num << std::endl;
    }
    //std::cout << std::endl;
    
    //std::cout << "--------Task--------:" << std::endl;
    //std::cout << "LCM = " << LCMPeriod << std::endl;
    for (size_t i = 0; i < SystemTask.size(); i++)
    {
        //std::cout << std::endl;
        //std::cout << "     Task " << i << std::endl;
        //std::cout << "Period: " << SystemTask[i]->Period << std::endl;
        //std::cout << "Time: " << SystemTask[i]->Time << std::endl;
        //std::cout << "Left: " << SystemTask[i]->Left << std::endl;
        //std::cout << "Right: " << SystemTask[i]->Right << std::endl;
        //std::cout << "Input Mes Count: " << SystemTask[i]->InMessage.size() << std::endl;
        //std::cout << "Output Mes Count: " << SystemTask[i]->OutMessage.size() << std::endl;
        //std::cout << "MesOut: " << std::endl;
        for (size_t j = 0; j < SystemTask[i]->MesOut.size(); j++)
        {
            std::shared_ptr<Message> p = SystemTask[i]->MesOut[j].lock();    
            if (p) 
            {
                //std::cout << p->Size << " ";
            } else 
            {
                //std::cout << "PROBLEMS" << std::endl;
            }
        }
        //std::cout << std::endl;

        //std::cout << "MesIN: " << std::endl;
        for (size_t j = 0; j < SystemTask[i]->MesIn.size(); j++)
        {
            std::shared_ptr<Message> p = SystemTask[i]->MesIn[j].lock();    
            if (p) 
            {
                //std::cout << p->Size << " ";
            } else 
            {
                //std::cout << "PROBLEMS" << std::endl;
            }
        }
        //std::cout << std::endl;
    }
    //std::cout << std::endl;

    //std::cout << "--------Message--------:" << std::endl;
    for (size_t i = 0; i < SystemMessage.size(); i++)
    {
        //std::cout << std::endl;
        //std::cout << "     Message " << i  << std::endl;
        //std::cout << "Src: " << SystemMessage[i]->Src->Time << std::endl;
        //std::cout << "Dest: " << SystemMessage[i]->Dest->Time << std::endl;
        //std::cout << "Size: " << SystemMessage[i]->Size << std::endl;
        //std::cout << "Bandwidth: " << SystemMessage[i]->Bandwidth << std::endl;
        //std::cout << "Dur: " << SystemMessage[i]->Dur << std::endl;
    }
    //std::cout << std::endl;
    //std::cout << "======================" << std::endl; 
    return;
}

void System:: PrintMessages()
{
    //std::cout << "--------Message--------:" << std::endl;
    for (size_t i = 0; i < SystemMessage.size(); i++)
    {
        //std::cout << std::endl;
        //std::cout << "     Message " << i  << std::endl;
        //std::cout << "Src: " << SystemMessage[i]->Src->Time << std::endl;
        //std::cout << "Dest: " << SystemMessage[i]->Dest->Time << std::endl;
        //std::cout << "Size: " << SystemMessage[i]->Size << std::endl;
        //std::cout << "Bandwidth: " << SystemMessage[i]->Bandwidth << std::endl;
        //std::cout << "Dur: " << SystemMessage[i]->Dur << std::endl;
    }
    //std::cout << std::endl;
    //std::cout << "-------- Context Message--------:" << std::endl;
    for (size_t i = 0; i < SystemCMessage.size(); i++)
    {
        //std::cout << std::endl;
        //std::cout << "     Message " << i  << std::endl;
        //std::cout << "Src: " << SystemCMessage[i]->Src->Time << std::endl;
        //std::cout << "Dest: " << SystemCMessage[i]->Dest->Time << std::endl;
        //std::cout << "Size: " << SystemCMessage[i]->Size << std::endl;
        //std::cout << "Bandwidth: " << SystemCMessage[i]->Bandwidth << std::endl;
        //std::cout << "Dur: " << SystemCMessage[i]->Dur << std::endl;
    }
    //std::cout << std::endl;
    //std::cout << "======================" << std::endl; 
    return;
}

void System:: PrintPC()
{
    //std::cout << "--------PC--------:" << std::endl;
    for (size_t i = 0; i < SystemPC.size(); i++)
    {
        //std::cout << std::endl;
        //std::cout << "       PC " << i << std::endl;
        for (const auto & Idx: SystemPC[i]->PlannedOnPC)
        {
            //std::cout << "Job Start = " << SystemJob[Idx]->Start << " Time = " << SystemJob[Idx]->Time << std::endl;
        }
    }
    //std::cout << std::endl;
}


std::map<int, std::string> AllColor = {
    {0, std::string("steelblue2")}, 
    {1, std::string("aqua")},
    {2, std::string("aquamarine")},
    {3, std::string("orangered1")},
    {4, std::string("x11purple")},
    {5, std::string("thistle1")},
    {6, std::string("violet")},
    {7, std::string("brown1")},
    {8, std::string("burlywood1")},
    {9, std::string("cadetblue1")},
    {10, std::string("chartreuse")},
    {11, std::string("chocolate")},
    {12, std::string("coral")},
    {13, std::string("cornflower")},
    {14, std::string("crimson")},
    {15, std::string("cyan")},
    {16, std::string("darkgoldenrod")},
    {17, std::string("darkkhaki")},
    {18, std::string("darkmagenta")},
    {19, std::string("lawngreen")},
    {20, std::string("lemonchiffon")},
    {21, std::string("lightblue")},
    {22, std::string("lightcoral")},
    {23, std::string("lightcyan")},
    {24, std::string("lightgoldenrod")},
    {25, std::string("lightgreen")},
    {26, std::string("lightpink")},
    {27, std::string("lightsalmon")},
    {28, std::string("lightseagreen")},
    {29, std::string("lightskyblue")},
    {30, std::string("lightslategray")},
    {31, std::string("lightsteelblue")},
    {32, std::string("lightyellow")},
    {33, std::string("lime")},
    {34, std::string("limegreen")},
    {35, std::string("linen")},
    {36, std::string("magenta")},
    {38, std::string("maroon")},
    {39, std::string("mediumaquamarine")},
    {40, std::string("mediumblue")},
    {41, std::string("mediumorchid")},
    {42, std::string("mediumseagreen")},
    {43, std::string("mediumslateblue")},
    {44, std::string("mediumspringgreen")},
    {45, std::string("mediumturquoise")}    
};

void System:: ExportToDotFile(char * filename)
{
    std::ofstream file;
    file.open(filename);
    std::string BaseInput = 
    "digraph G {\n\n"
    "fontname=\"Helvetica,Arial,sans-serif\"\n"
    "node [fontname=\"Helvetica,Arial,sans-serif\"]\n"
    "edge [fontname=\"Helvetica,Arial,sans-serif\"]\n"
    "graph [rankdir = \"LR\"];\n"
    "node [fontsize = \"20\" shape = \"ellipse\"];\n";
    int MesNum = 0;
    std::string CMes, CMesContact;
        
    std::string MesContact;
    std::string NewMessage;
    file << BaseInput << std::endl;
    for (size_t i = 0; i < SystemPC.size(); i++)
    {
        std::string NewSubgraphBase;
        if (!i || SystemPC[i]->ModNum != SystemPC[i - 1]->ModNum)
        {
            if (i)
                NewSubgraphBase += "    }\n";    
            NewSubgraphBase += 
            "    subgraph cluster_Mod_" + std::to_string(SystemPC[i]->Num) + "{\n" + 
            "        color =" + AllColor[(10 + SystemPC[i]->ModNum) % AllColor.size()] + "\n" +
            "        style = bold\n";
        }
        NewSubgraphBase += 
        "        subgraph cluster_";
        NewSubgraphBase += std::to_string(i);
        NewSubgraphBase +=
        "{\n"
        "           node [style=filled]\n"
        "           color = black\n";
        std::string NewJobLine = 
        "        "; 
        
        file <<  NewSubgraphBase << std::endl; 
        for (size_t j = 0; j < SystemPC[i]->PlannedOnPC.size(); j++)
        {
            std::string NewSubgraph = 
            "\n            \"node" + std::to_string(SystemPC[i]->PlannedOnPC[j]);;
            if (NewJobLine != 
                "        ") NewJobLine += " -> ";
            NewJobLine += "node" + std::to_string(SystemPC[i]->PlannedOnPC[j]);; 
        
            NewSubgraph +=
            "\" [\n"
            "               label = \"";
            std::ostringstream streamObj1;
            std::ostringstream streamObj2;
            std::ostringstream streamObj3;
            streamObj1 << SystemJob[SystemPC[i]->PlannedOnPC[j]]->Start;
            NewSubgraph += "Start Time : " + streamObj1.str() + " | ";
            streamObj2 << SystemJob[SystemPC[i]->PlannedOnPC[j]]->Start + SystemJob[SystemPC[i]->PlannedOnPC[j]]->Time;
            NewSubgraph += "End Time : " + streamObj2.str() + " | ";
            streamObj3 << SystemJob[SystemPC[i]->PlannedOnPC[j]]->Time;
            NewSubgraph += "Duration : " + streamObj3.str() + "\"\n";
            NewSubgraph +=
            "               shape = \"record\"\n"
            "               color = ";
            
            NewSubgraph += AllColor[SystemJob[SystemPC[i]->PlannedOnPC[j]]->NumOfTask] + "\n";  
            NewSubgraph += "            ]";
            file << NewSubgraph << std::endl;
            for (const auto & Idx: SystemJob[SystemPC[i]->PlannedOnPC[j]]->OutMessage)
            {
                if (SystemJob[SystemPC[i]->PlannedOnPC[j]]->JobPC->ModNum == SystemJob[Idx]->JobPC->ModNum)
                {
                    continue;
                }
                NewMessage += "    \"message" + std::to_string(MesNum) + "\" [\n";
                std::ostringstream streamObj1;
                std::ostringstream streamObj2;
                streamObj1 << JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(
                                           SystemJob[SystemPC[i]->PlannedOnPC[j]],
                                           SystemJob[Idx])]->Dur;
                streamObj2 << JobMessage[std::pair<std::shared_ptr<Job>, std::shared_ptr<Job>>(
                                           SystemJob[SystemPC[i]->PlannedOnPC[j]],
                                           SystemJob[Idx])]-> Bandwidth;
                NewMessage +=
                "       label = \"Dur : " + streamObj1.str() + " | " + "Band : " + streamObj2.str() + "\"\n"
                "       shape = \"ellipse\""
                "   ]\n";
                MesContact += "    node" + std::to_string(SystemPC[i]->PlannedOnPC[j]) +
                                          " -> " + "message" + std::to_string(MesNum) + " [color =" + AllColor[SystemJob[SystemPC[i]->PlannedOnPC[j]]->NumOfTask] +" style = bold]\n" + 
                                          "    message" + std::to_string(MesNum) + " -> " + "node" + std::to_string(Idx) + " [color =" + AllColor[SystemJob[Idx]->NumOfTask] +" style = bold]\n";
                MesNum++;
            }
        }
        

        file << "\n    " + NewJobLine + "\n" << std::endl; 
        file << "    " + std::string("        label = \"PC #") + std::to_string(i + 1) + "\"\n" << std::endl;   
        file << "        }" << std::endl;

    }
    MesNum = 0;
        
    for (const auto & Mes: SystemCMessage)
    {
        
        CMes += "    \"contextMessage" + std::to_string(MesNum) + "\" [\n";
        std::ostringstream streamObj1;
        std::ostringstream streamObj2;
        streamObj1 << Mes->Dur;
        streamObj2 << Mes-> Bandwidth;
        CMes +=
        "       label = \"Dur : " + streamObj1.str() + " | " + "Band : " + streamObj2.str() + "\"\n"
        "       shape = \"ellipse\""
        "       color = lightpink\n"
        "       style = filled"
        "   ]\n";
        CMesContact += "    node" + std::to_string(std::find(SystemJob.begin(), SystemJob.end(), Mes->Src) - SystemJob.begin()) +
                                  " -> " + "contextMessage" + std::to_string(MesNum) +
                                  " -> " + "node" + std::to_string(std::find(SystemJob.begin(), SystemJob.end(), Mes->Dest) - SystemJob.begin()) +"\n";
        MesNum++;
    }
    
    file << "    }" << std::endl; 
    file << CMes << std::endl;
    file << CMesContact << std::endl;
    
    file << NewMessage << std::endl;
    file << MesContact << std::endl;
    file <<"}\n" <<std::endl;
    return;
}

// загрузка из .txt файла, сейчас не используется
/*
System::System (char * FileName)
{
    std::ifstream InputFile(FileName);
    int ModNum, PCNum, k = 0;
    int TaskNum, Period, Time, Left, Right, NMessage, Num;
    double CMessageSize;
    int MessageNum, Src, Dest, Size;
    std::vector<int> PeriosVector;
    InputFile >> ModNum;
    for (int i = 0; i < ModNum; i++)
    {
        InputFile >> PCNum;
        for (int j=0; j< PCNum; j++)
        {
            SystemPC.push_back(std::make_shared<PC>(i));
            SystemPC[k]->Num = k;
            k++;
        }        
    }
    InputFile >> TaskNum;
    for (int i = 0; i < TaskNum; i++)
    {
        InputFile >> Period >> Time >> Left >> Right >>CMessageSize;
        PeriosVector.emplace_back(Period);
        SystemTask.push_back(std::make_shared<Task>(
                             Period, Time, Left, Right, CMessageSize));
        SystemTask[i]->InitLeft = SystemTask[i]->Left;
        SystemTask[i]->MaxLeft = SystemTask[i]->Left;
        SystemTask[i]->MinLeft = SystemTask[i]->Left;
        SystemTask[i]->InitRight = SystemTask[i]->Right;
    }
    LCMPeriod = LCM(PeriosVector);
    InputFile >> BTotal;    
    InputFile >> MessageNum;
    
    for (int i = 0; i < MessageNum; i++)
    {
        InputFile >> Src >> Dest >> Size;
        SystemMessage.push_back(std::make_shared<Message>(
                                SystemTask[Src], SystemTask[Dest], Size));
        //SystemMessage[i]->IsPlanned = true;
        SystemTask[Src]->OutMessage.emplace_back(Dest);
        SystemTask[Src]->MesOut.push_back(std::weak_ptr<Message>(SystemMessage[i]));
        SystemTask[Dest]->InMessage.emplace_back(Src);
        SystemTask[Dest]->MesIn.emplace_back(std::weak_ptr<Message>(SystemMessage[i]));
        SystemMessage[i]->DestNum = Dest;
        SystemMessage[i]->SrcNum = Src;
        SystemMessage[i]->StabilityCoef = StableCoef(LCMPeriod / SystemTask[Src]->Period);
    }
    
    return;
}
*/