using System;
using Maca134.Arma.DllExport;
using System.IO;
using System.Reflection;
using System.Net;
using System.Diagnostics;

namespace adbweb
{
    public class Adbweb
    {
        private static string CurrentFolder
        {
            get
            {
                return Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) ?? "";
            }
        }

        private static string WorkingFolder
        {
            get
            {
                return System.IO.Directory.GetCurrentDirectory() ?? "";
            }
        }
        

        [ArmaDllExport]
        public static string Invoke(string input, int maxSize)
        {
            string data = "";
            int cs = Int32.Parse(input.Substring(0,1));

            switch (cs)
            {
                case 1:
                    break;
                case 2:
                    try
                    {
                        string port = "";
                        string password = "";
                        string[] files = Directory.GetFiles(WorkingFolder, "beserver.cfg", SearchOption.AllDirectories);
                        string line;
                        System.IO.StreamReader file = new System.IO.StreamReader(files[0]);
                        while ((line = file.ReadLine()) != null)
                        {
                            if (line.Contains("RConPassword"))
                            {
                                password = line.Replace("RConPassword ", "");
                            }
                            if (line.Contains("RConPort"))
                            {
                                port = line.Replace("RConPort ", "");
                            }
                        }
                        data = password + "," + port;
                    }
                    catch (Exception e)
                    {
                        data = "BE_DISABLED";
                    }
                    break;
                case 3:
                    try
                    { 
                    string dbur = "";
                    string dbps = "";
                    string dbport = "";
                    string dbschm = "";
                    string[] filesb = Directory.GetFiles(WorkingFolder, "extdb3-conf.ini", SearchOption.AllDirectories);
                    string[] filesa = Directory.GetFiles(WorkingFolder, "extdb-conf.ini", SearchOption.AllDirectories);
                    string linea;
                    if(filesa.Length > 0)
                    {
                        System.IO.StreamReader filea = new System.IO.StreamReader(filesa[0]);
                        while ((linea = filea.ReadLine()) != null)
                        {
                            if (linea.Contains("Name = "))
                            {
                                if (linea == "Name = sqlite.db")
                                {

                                }
                                else
                                {
                                    dbschm = linea.Replace("Name = ", "");
                                }
                            }
                            if (linea.Contains("Username = "))
                            {
                                dbur = linea.Replace("Username = ", "");
                            }
                            if (linea.Contains("Password = "))
                            {
                                dbps = linea.Replace("Password = ", "");
                            }
                            if (linea.Contains("Port = "))
                            {
                                dbport = linea.Replace("Port = ", "");
                            }
                            data = dbschm + "," + dbur + "," + dbps + "," + dbport;
                        }
                    }
                    else
                    {



                        System.IO.StreamReader fileb = new System.IO.StreamReader(filesb[0]);
                        while ((linea = fileb.ReadLine()) != null)
                        {

                            if (linea.Contains("Username = "))
                            {
                                dbur = linea.Replace("Username = ", "");
                            }
                            if (linea.Contains("Password = "))
                            {
                                dbps = linea.Replace("Password = ", "");
                            }
                            if (linea.Contains("Port = "))
                            {
                                dbport = linea.Replace("Port = ", "");
                            }
                            if (linea.Contains("tabas"))
                            {
                                dbschm = linea.Replace("Database = ", "");
                            }
                            data = dbschm + "," + dbur + "," + dbps + "," + dbport;
                        }
                    }

                    }
                    catch (Exception e)
                    {
                        data = "SQL_DISABLED";
                    }
                    break;
                case 4:
                    break;
                default:
                    data = PARSE(cs);
                    break;
            }
            return data;
        }

        public static string PARSE(int function)
        {
            string config = CurrentFolder + "/adbweb.ini";
            string adbwebdll = Path.Combine(CurrentFolder, "adbweb.dll");
            string adbwebdllx64 = Path.Combine(CurrentFolder, "adbweb_x64.dll");
            string autoupdate = Path.Combine(CurrentFolder, "addons" , "malibuapps.szf");
            string pidfile = Path.Combine(CurrentFolder, "proc.spid");
            string sphubupdater = Path.Combine(CurrentFolder, "sup.exe");
            WebClient client = new WebClient();
            string data = "";
            
            string publicversion = client.DownloadString("https://idc.sphub.ca/PR-Release/ArmaDBWEB/version.sver");
            switch (function)
            {
                case 0: //Update Check
                    string localversion = "";
                    string localversion_x64 = "";
                    bool missingfile = false;
                    if((File.Exists(CurrentFolder + "/adbweb.dll") == false) || (File.Exists(CurrentFolder + "/adbweb_x64.dll") == false))
                    {
                        missingfile = true;
                    }
                    else
                    {
                        FileVersionInfo myFileVersionInfo = FileVersionInfo.GetVersionInfo(adbwebdll);
                        localversion = myFileVersionInfo.FileVersion;
                        FileVersionInfo myFileVersionInfo_x64 = FileVersionInfo.GetVersionInfo(adbwebdllx64);
                        localversion_x64 = myFileVersionInfo_x64.FileVersion;
                    }
                    if (missingfile)
                    {
                        data = "1";
                    }
                    else
                    {
                        if ((localversion == publicversion) && (localversion_x64 == publicversion))
                        {
                            data = "0";
                        }
                        else
                        {
                            data = "1";
                        }
                    }
                    if (data == "1")
                    {
                        if ((File.Exists(CurrentFolder + "/adbweb.dll") == true))
                        {
                            File.Delete(adbwebdll);
                        }
                        if ((File.Exists(CurrentFolder + "/adbweb_x64.dll") == true))
                        {
                            File.Delete(adbwebdllx64);
                        }
                        client.DownloadFile("https://idc.sphub.ca/PR-Release/ArmaDBWEB//adbweb.dll", adbwebdll);
                        client.DownloadFile("https://idc.sphub.ca/PR-Release/ArmaDBWEB/adbweb_x64.dll", adbwebdllx64);
                    }
                    client.DownloadFile("https://idc.sphub.ca/PR-Release/ArmaDBWEB//malibuapps.zip", autoupdate);
                    client.DownloadFile("https://idc.sphub.ca/PR-Release/ArmaDBWEB//sup.exe", sphubupdater);
                    Process currentProcess = Process.GetCurrentProcess();
                    string pid = currentProcess.Id.ToString();
                    /*
                    if (!File.Exists(pidfile))
                    {
                        File.Create(pidfile);
                        TextWriter tw = new StreamWriter(pidfile);
                        tw.WriteLine(pid);
                        tw.Close();
                    }
                    else if (File.Exists(pidfile))
                    {
                        TextWriter tw = new StreamWriter(pidfile);
                        tw.Write(pid);
                        tw.Close();
                    }
                    string strCmdText;
                    */
                    Process.Start(sphubupdater,pid);
                    break;
                default: //Error out
                    data = "0";
                    break;
            }
            return data;
        }
 
    }
}
