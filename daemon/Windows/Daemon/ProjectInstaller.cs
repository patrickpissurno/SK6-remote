using Microsoft.Win32.TaskScheduler;
using System;
using System.ComponentModel;
using System.Configuration.Install;
using System.IO;
using System.Management;
using System.Linq;

namespace Daemon
{
    [RunInstaller(true)]
    public class ProjectInstaller : Installer
    {
        internal static readonly string TASK_NAME = "SK6-Remote";

        public ProjectInstaller() : base()
        {
            this.AfterInstall += ProjectInstaller_AfterInstall;
            this.AfterUninstall += ProjectInstaller_AfterUninstall;
            this.BeforeUninstall += ProjectInstaller_BeforeUninstall;
        }

        private void ProjectInstaller_BeforeUninstall(object sender, InstallEventArgs e)
        {
            using (TaskService ts = new TaskService())
            {
                var task = ts.FindTask(TASK_NAME);
                if(task != null)
                    task.Stop();
            }
        }

        private void ProjectInstaller_AfterUninstall(object sender, InstallEventArgs e)
        {
            using (TaskService ts = new TaskService())
            {
                ts.RootFolder.DeleteTask(TASK_NAME, false);
            }
        }

        public void ProjectInstaller_AfterInstall(object sender, InstallEventArgs e)
        {
            string path = this.Context.Parameters["targetdir"];

            // Get the service on the local machine
            using (TaskService ts = new TaskService())
            {
                // Create a new task definition and assign properties
                TaskDefinition td = ts.NewTask();
                td.RegistrationInfo.Description = "Makes your LG SK6 soundbar remote controller work with your computer";

                ManagementObjectSearcher searcher = new ManagementObjectSearcher("SELECT UserName FROM Win32_ComputerSystem");
                ManagementObjectCollection collection = searcher.Get();
                string username = (string)collection.Cast<ManagementBaseObject>().First()["UserName"];

                td.Principal.UserId = username;
                td.Principal.LogonType = TaskLogonType.InteractiveToken;

                // Create a trigger that will fire the task at this time every other day
                td.Triggers.Add(new LogonTrigger());

                // Create an action that will launch Notepad whenever the trigger fires
                td.Actions.Add(new ExecAction(Path.Combine(path, "Daemon.exe"), null));

                td.Settings.Compatibility = TaskCompatibility.V2;
                td.Settings.UseUnifiedSchedulingEngine = true;

                td.Settings.AllowDemandStart = true;
                td.Settings.DisallowStartIfOnBatteries = false;
                td.Settings.Enabled = true;
                td.Settings.RestartCount = 99999;
                td.Settings.StopIfGoingOnBatteries = false;
                td.Settings.RestartInterval = TimeSpan.FromMinutes(1);
                td.Settings.Priority = System.Diagnostics.ProcessPriorityClass.BelowNormal;
                td.Settings.RunOnlyIfIdle = false;
                td.Settings.ExecutionTimeLimit = new TimeSpan();
                td.Settings.MultipleInstances = TaskInstancesPolicy.StopExisting;

                // Register the task in the root folder
                var task = ts.RootFolder.RegisterTaskDefinition(TASK_NAME, td, TaskCreation.CreateOrUpdate, username, null, TaskLogonType.InteractiveToken);

                //Start
                task.Run();
            }
        }
    }
}
