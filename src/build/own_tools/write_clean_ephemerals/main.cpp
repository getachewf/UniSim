#include "../common/common.h"

void turnSlashes(QString &path) {
     #if defined (__WIN32__)
       path = path.replace("/", "\\");
     #endif
}

QString cleanCmd(QDir dir, QString folderName) {
    QString folderPath = dir.absolutePath() + "/" + folderName;
    turnSlashes(folderPath);
    #if defined (__WIN32__)
       return "call clean_folder " + folderPath + "\n";
    #else
       return "bash clean_folder.sh " + folderPath + "\n";
    #endif
}

QString delCmd(QDir dir, QString fileName) {
    QString filePath = dir.absolutePath() + "/" + fileName;
    turnSlashes(filePath);
    #if defined (__WIN32__)
      return "if exist " + filePath +" del " + filePath + " /Q\n";
    #else
      return "if [ -f " + filePath +" ]; then rm -f " + filePath + "; fi \n";
    #endif
}

int main(int argc, char *argv[])
{
    //QString filePath = "../win/clean_ephemerals.bat";
    QString filePath = "../ubuntu/clean_ephemerals.sh";
    #if defined (__WIN32__)
           filePath = "../win/clean_ephemerals.bat";
    #endif

    openFile(filePath);

    writeTimeStamp("# ");
    write(" \n ");

    #if defined (__WIN32__)
       writeTimeStamp("@rem ");
       write("@echo off\n");
    #endif

    write(cleanCmd(getSourceDir(), "build/own_tools/update_settings"));
    write(cleanCmd(getSourceDir(), "build/own_tools/write_clean_ephemerals"));
    write(cleanCmd(getSourceDir(), "build/own_tools/write_plugins_project"));
    write(cleanCmd(getSourceDir(), "build/own_tools/write_unisim_root"));


    write(cleanCmd(getSourceDir(), "applications/unisim"));
    write(cleanCmd(getSourceDir(), "applications/unisim/test"));

    QStringList pluginFolders = getPluginFolders();
    for (int i = 0; i < pluginFolders.size(); ++i)
        write(cleanCmd(getPluginsDir(), pluginFolders[i]));

    write(delCmd(getSourceDir(), "makefile.*"));
    write(delCmd(getPluginsDir(), "makefile.*"));

    #if defined (__WIN32__)
       write("@echo on\n");
       write("@echo *\n");
       write("@echo * UniSim project cleaned for all intermediate files *\n");
       write("@echo *\n");
   #else
      write(" echo \"* UniSim project cleaned for all intermediate files * \" \n");
    #endif

    closeFile();
    return 0;
}
