#include "serializer.h"

//This file is going to be autogenerated
#define DISTANCE_MAIN_ID 23
#define DISTANCE_SUB_ID 15

void Serializer::initializeIds() {
    int slot = 0;
    QString pathprefix = QDir(qApp->applicationDirPath()).absoluteFilePath("../../../CSV2CPP/");
    slot = loadInCSVSpec(slot, pathprefix+"strategy_overview.csv");
    slot = loadInCSVSpec(slot, pathprefix+"CAN_overview.csv");
    slot = loadInCSVSpec(slot, pathprefix+"combined_overview.csv");

    //fill all the datafields with zeroes
    char zero[1] = {0};
    for(int i = 0; i < dataStruct.length(); i++) {
        for (int j = 0; j < dataStruct.at(i).second.length(); j++) {
            Type type = dataStruct.at(i).second.at(j).type;
            QByteArray zeroes;
            for(int k = 0; k < serialize[type]->getSize(); k++) { //add bytes of only zeroes, depending on the length of the type that is stored here
                zeroes.append(zero,1);
            }
            if (zeroes.length() != serialize[type]->getSize()) {
                qDebug() << "Warning: size isn't initialized well";
            }
            dataStruct[i].second[j].dataSize = serialize[type]->getSize();
            dataStruct[i].second[j].dataField = zeroes;
        }
    }
    restructureDatastructForDatablocks(2);
    restructureDatastructForDatablocks(4);
    restructureDatastructForDatablocks(5);
    restructureDatastructForDatablocks(6);
}

void Serializer::restructureDatastructForDatablocks(int id) {
    int length = 0;
    for(int i = 0; i < dataStruct.at(lookUp[id]).second.length(); i++) {
        length += serialize[dataStruct.at(lookUp[id]).second.at(i).type]->getSize();
    }
    qDebug() << "calculated Length" << length;
    dataStruct[lookUp[id]].second[0].dataSize = length;
    dataStruct[lookUp[id]].second[0].dataField.clear();
    QByteArray zeroes;
    char zero[1] = { 0 };
    for(int k = 0; k < length; k++) { //add bytes of only zeroes, depending on the length of the type that is stored here
        zeroes.append(zero,1);
    }
    qDebug() << "Lengths " << id << length;
    dataStruct[lookUp[id]].second[0].dataField = zeroes;
    for (int i = 1; i < dataStruct[lookUp[id]].second.length(); i++) {
        dataStruct[lookUp[id]].second[i].dataSize = 0;
        dataStruct[lookUp[id]].second[i].dataField.clear();
    }
}

int Serializer::loadInCSVSpec(int i, QString file) {
    //interpret the canspec
    qDebug() << file;
    QFile inputFile(file);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file: " << inputFile.errorString();
        return i;
    }
    qDebug() << "hoi";
   QTextStream in(&inputFile);
   in.readLine();
   in.readLine();
   while (!in.atEnd()) {
      QString line = in.readLine();
      QStringList splitted = line.split(";");
      quint32 id = splitted.at(0).toInt();
      QString name = splitted.at(1);
      QStringList fieldNames = splitted.at(2).split(",");
      QStringList types = splitted.at(3).split(",");

      QStringList toVisList=  splitted.at(13).split(","); //get the id's that
      QStringList toStratList = splitted.at(12).split(","); //get the id's that
      if (splitted.at(14) != "check") { //if this message should be written to the database
          QList<LastDataStruct> list; //TODO autogenerate this
          for (int j = 0; j < fieldNames.length(); j++) { //for each field
              bool toVis = false;
              for (int k = 0; k < toVisList.length(); k++) {
                  toVisList[k].remove(" ");
                  if(toVisList.at(k).toInt() == j && toVisList.at(k) != "") { //check whether it should be sent to the strat system
                      toVis = true;
                      break;
                  }
              }
              bool toStrat = false;
              for (int k = 0; k < toStratList.length(); k++) {
                  toStratList[k].remove(" ");
                  if(toStratList.at(k).toInt() == j && toStratList.at(k) != "") { //check whether it should be sent to the strat system
                      toStrat = true;
                      break;
                  }
              }

              Type type;
              types[j].remove(" ");
              QString thisType = types[j];
              if (thisType == "uint8_t")
                  type = UInt8;
              else if (thisType == "uint16_t")
                  type = UInt16;
              else if (thisType == "uint32_t")
                  type = UInt32;
              else if (thisType == "uint64_t")
                  type = UInt64;
              else if (thisType == "float")
                  type = Float;
              else if (thisType == "int8_t")
                  type = Int8;
              else if (thisType == "int16_t")
                  type = Int16;
              else if (thisType == "int32_t")
                  type = Int32;
              else if (thisType == "int64_t")
                  type = Int64;
              else
                  type = Other;

              name.remove(" ");
              fieldNames[j].remove(" ");
              list.append(LastDataStruct(type, toVis, toStrat, name+"_"+fieldNames.at(j)));
              //qDebug() << id << j << type << "," << toVis << "," << toStrat << "," << name+"_"+fieldNames.at(j);
          }
          dataStruct.append(qMakePair(id,list));
          lookUp[id] = i; //so this id is to be found at index i of the list
          i++; //increment for the id that is going to be written to the datastruct
      }
   }
    return i;
}

void Serializer::showSpec() {
    qDebug() << "Database spec:";
    int count = 2;
    qDebug() << "1,timestamp,5";
    qDebug() << "2,distance,0";
    for(int i = 10; i < dataStruct.length(); i++) {
        for (int j = 0; j < dataStruct.at(i).second.length(); j++) {
            count++;
            qDebug() << count <<"," <<dataStruct.at(i).second.at(j).name<< "," << dataStruct.at(i).second.at(j).type;
        }
    }
    count = 2;
    int lengthCount = 12;
    qDebug() << " ";
    qDebug() << "Visualizer spec:";
    qDebug() << "1,timestamp,5";
    qDebug() << "2,distance,0";
    for(int i = 10; i < dataStruct.length(); i++) {
        for (int j = 0; j < dataStruct.at(i).second.length(); j++) {
            if (dataStruct.at(i).second.at(j).toVis) {
                count++;
                lengthCount += serialize[dataStruct.at(i).second.at(j).type]->getSize();
                qDebug() << count <<"," <<dataStruct.at(i).second.at(j).name<< "," << dataStruct.at(i).second.at(j).type;
            }
        }
    }
    //qDebug() << "length" << lengthCount;
    visMsgLength = lengthCount;
    lengthCount = 12;
    count = 2;
    qDebug() << " ";
    qDebug() << "To strategy system spec:";
    qDebug() << "1,timestamp,5";
    qDebug() << "2,distance,0";
    for(int i = 10; i < dataStruct.length(); i++) {
        for (int j = 0; j < dataStruct.at(i).second.length(); j++) {
            if (dataStruct.at(i).second.at(j).toStrat) {
                count++;
                lengthCount += serialize[dataStruct.at(i).second.at(j).type]->getSize();
                qDebug() << count <<"," <<dataStruct.at(i).second.at(j).name<< "," << dataStruct.at(i).second.at(j).type;
            }
        }
    }
    //qDebug() << "length" << lengthCount;
    stratMsgLength = lengthCount;

    count = 0;
    qDebug() << " ";
    qDebug() << "Master forecast regels";
    for (int j = 0; j < dataStruct.at(lookUp[2]).second.length(); j++) {
        count++;
        qDebug() << count <<"," <<dataStruct.at(lookUp[2]).second.at(j).name<< "," << dataStruct.at(lookUp[2]).second.at(j).type;
    }

    count = 0;
    qDebug() << " ";
    qDebug() << "Short term strategy regels";
    for (int j = 0; j < dataStruct.at(lookUp[4]).second.length(); j++) {
        count++;
        qDebug() << count <<"," <<dataStruct.at(lookUp[4]).second.at(j).name<< "," << dataStruct.at(lookUp[4]).second.at(j).type;
    }

    count = 0;
    qDebug() << " ";
    qDebug() << "Long term strategy regels";
    for (int j = 0; j < dataStruct.at(lookUp[5]).second.length(); j++) {
        count++;
        qDebug() << count <<"," <<dataStruct.at(lookUp[5]).second.at(j).name<< "," << dataStruct.at(lookUp[5]).second.at(j).type;
    }

    count = 0;
    qDebug() << " ";
    qDebug() << "Parameters";
    for (int j = 0; j < dataStruct.at(lookUp[6]).second.length(); j++) {
        count++;
        qDebug() << count <<"," <<dataStruct.at(lookUp[6]).second.at(j).name<< "," << dataStruct.at(lookUp[6]).second.at(j).type;
    }
}
