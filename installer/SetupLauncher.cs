using System;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;
internal static class SetupLauncher {
    [STAThread] private static void Main() {
        string root = AppDomain.CurrentDomain.BaseDirectory;
        string script = Path.Combine(root,"installer","Install-Syspect.ps1");
        try {
            var start = new ProcessStartInfo("powershell.exe", "-NoLogo -NoProfile -ExecutionPolicy Bypass -File \"" + script + "\"");
            start.WorkingDirectory=root; start.UseShellExecute=false; start.CreateNoWindow=true;
            start.RedirectStandardError=true; start.RedirectStandardOutput=true;
            using(var process=Process.Start(start)) {
                var output=process.StandardOutput.ReadToEndAsync();
                var errors=process.StandardError.ReadToEndAsync();
                process.WaitForExit();
                if(process.ExitCode!=0) MessageBox.Show(errors.Result,"Syspect setup failed",MessageBoxButtons.OK,MessageBoxIcon.Error);
                else MessageBox.Show("Syspect 1.1.0 installed. Open Syspect from your desktop.","Syspect");
            }
        } catch(Exception ex) { MessageBox.Show(ex.Message,"Syspect setup failed"); }
    }
}
