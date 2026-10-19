using System;
using System.Diagnostics;
using System.IO;

internal static class Program
{
    [STAThread]
    private static void Main()
    {
        string root = AppDomain.CurrentDomain.BaseDirectory;
        string host = Path.Combine(root, "desktop", "SyspectHost.ps1");
        if (!File.Exists(host))
        {
            System.Windows.Forms.MessageBox.Show(
                "SyspectHost.ps1 is missing. Reinstall Syspect.",
                "Syspect",
                System.Windows.Forms.MessageBoxButtons.OK,
                System.Windows.Forms.MessageBoxIcon.Error);
            return;
        }

        var psi = new ProcessStartInfo
        {
            FileName = "powershell.exe",
            Arguments = "-NoLogo -NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File \"" + host + "\"",
            WorkingDirectory = root,
            UseShellExecute = false,
            CreateNoWindow = true,
            WindowStyle = ProcessWindowStyle.Hidden
        };
        try { Process.Start(psi); }
        catch (Exception ex)
        {
            System.Windows.Forms.MessageBox.Show(
                "Syspect could not start.\n\n" + ex.Message,
                "Syspect",
                System.Windows.Forms.MessageBoxButtons.OK,
                System.Windows.Forms.MessageBoxIcon.Error);
        }
    }
}
