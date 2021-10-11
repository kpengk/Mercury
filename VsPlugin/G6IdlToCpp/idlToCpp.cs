using EnvDTE;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using System;
using System.ComponentModel.Design;
using System.Globalization;
using System.IO;
using System.Text;
using Task = System.Threading.Tasks.Task;

namespace G6IdlToCpp
{
    /// <summary>
    /// Command handler
    /// </summary>
    internal sealed class idlToCpp
    {
        /// <summary>
        /// Command ID.
        /// </summary>
        public const int CommandId = 0x0100;

        /// <summary>
        /// Command menu group (command set GUID).
        /// </summary>
        public static readonly Guid CommandSet = new Guid("806f6b05-dff1-46eb-baca-7ccf5c277bc7");

        /// <summary>
        /// VS Package that provides this command, not null.
        /// </summary>
        private readonly AsyncPackage package;

        private IVsOutputWindowPane pane;

        /// <summary>
        /// Initializes a new instance of the <see cref="idlToCpp"/> class.
        /// Adds our command handlers for menu (commands must exist in the command table file)
        /// </summary>
        /// <param name="package">Owner package, not null.</param>
        /// <param name="commandService">Command service to add command to, not null.</param>
        private idlToCpp(AsyncPackage package, OleMenuCommandService commandService)
        {
            this.package = package ?? throw new ArgumentNullException(nameof(package));
            commandService = commandService ?? throw new ArgumentNullException(nameof(commandService));

            var menuCommandID = new CommandID(CommandSet, CommandId);
            var menuItem = new MenuCommand(this.Execute, menuCommandID);
            commandService.AddCommand(menuItem);
        }

        /// <summary>
        /// Gets the instance of the command.
        /// </summary>
        public static idlToCpp Instance
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the service provider from the owner package.
        /// </summary>
        private Microsoft.VisualStudio.Shell.IAsyncServiceProvider ServiceProvider
        {
            get
            {
                return this.package;
            }
        }

        /// <summary>
        /// Initializes the singleton instance of the command.
        /// </summary>
        /// <param name="package">Owner package, not null.</param>
        public static async Task InitializeAsync(AsyncPackage package)
        {
            // Switch to the main thread - the call to AddCommand in idlToCpp's constructor requires
            // the UI thread.
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync(package.DisposalToken);

            OleMenuCommandService commandService = await package.GetServiceAsync(typeof(IMenuCommandService)) as OleMenuCommandService;
            Instance = new idlToCpp(package, commandService);
        }

        /// <summary>
        /// This function is the callback used to execute the command when the menu item is clicked.
        /// See the constructor to see how the menu item is associated with this function using
        /// OleMenuCommandService service and MenuCommand class.
        /// </summary>
        /// <param name="sender">Event sender.</param>
        /// <param name="e">Event args.</param>
        private void Execute(object sender, EventArgs e)
        {
            ThreadHelper.ThrowIfNotOnUIThread();

            ShowMessage("", true);

            DTE dte = Package.GetGlobalService(typeof(SDTE)) as DTE;
            string filename = dte.ActiveDocument.FullName;

            if (!IsInterfaceFile(filename))
            {
                ShowMessage("Not an interface file.");
                return;
            }

            // get generator path
            G6IdlToCppPackage idlToCppPackage = this.package as G6IdlToCppPackage;
            string generatorDir = idlToCppPackage.OptionGeneratorPath;
            if (generatorDir == null || generatorDir.Length <= 0)
            {
                ShowMessage("Please set the generator path in the \"Tools -> Options -> IDL To Cpp\" dialog box.");
                return;
            }
            if (!generatorDir.EndsWith("/") && !generatorDir.EndsWith("\\"))
            {
                generatorDir += "/";
            }
            string generatorPath = generatorDir + "interface_generator_app.exe";
            if (!System.IO.File.Exists(generatorPath))
            {
                ShowMessage(string.Format(CultureInfo.CurrentCulture, "Generator does not exist. path: {0}\n" +
                    "Please set the generator path in the \"Tools -> Options -> IDL To Cpp\" dialog box.", generatorPath));
                return;
            }

            // get output path
            string outPath = idlToCppPackage.OptionOutputPath;
            if (outPath.Length <= 0)
            {
                outPath = "idl-out";
            }

            if (!outPath.StartsWith("/") && !outPath.StartsWith("\\") && !outPath.Contains(":"))
            {
                string currentPath = dte.Solution.FullName;
                if (!IsDir(currentPath))
                {
                    currentPath = Path.GetDirectoryName(currentPath);
                }
                outPath = string.Format(CultureInfo.CurrentCulture, "{0}\\{1}", currentPath, outPath);
            }

            // run
            System.Diagnostics.Process proc = new System.Diagnostics.Process();
            proc.StartInfo.FileName = generatorPath;
            proc.StartInfo.Arguments = string.Format(CultureInfo.CurrentCulture, "-i {0} -d {1} -o {2}",
                filename, Path.GetDirectoryName(filename), outPath);
            proc.StartInfo.RedirectStandardError = true;
            proc.StartInfo.RedirectStandardOutput = true;
            proc.StartInfo.UseShellExecute = false;
            proc.StartInfo.CreateNoWindow = true;
            proc.Start();

            string outStr = proc.StandardOutput.ReadToEnd();
            string errStr = proc.StandardError.ReadToEnd();
            proc.Close();

            string message = string.Format(CultureInfo.CurrentCulture, "{0}\n{1}", outStr, errStr);
            ShowMessage(message);
        }

        private void CreatePane(Guid paneGuid, string title, bool visible, bool clearWithSolution)
        {
            ThreadHelper.ThrowIfNotOnUIThread();

            IVsOutputWindow output = (IVsOutputWindow)Package.GetGlobalService(typeof(SVsOutputWindow));

            // Create a new pane.  
            output.CreatePane(
                ref paneGuid,
                title,
                Convert.ToInt32(visible),
                Convert.ToInt32(clearWithSolution));

            // Retrieve the new pane.
            output.GetPane(ref paneGuid, out pane);
        }

        private void ShowMessage(string msg, bool clear = false)
        {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (pane == null)
            {
                CreatePane(new Guid(), "IDL To CPP", true, false);
            }
            if (clear)
            {
                pane.Clear();
            }
            pane.OutputString(msg);
            pane.OutputString("\n");
        }

        private bool IsInterfaceFile(string filename)
        {
            string suffix = Path.GetExtension(filename);
            if (!suffix.Equals(".h") && !suffix.Equals(".g6idl"))
            {
                return false;
            }

            StreamReader sr = new StreamReader(filename, Encoding.Default);
            string content;
            while ((content = sr.ReadLine()) != null)
            {
                if (content.ToString().Contains("interface"))
                {
                    return true;
                }
            }

            return false;
        }

        public static bool IsDir(string filepath)
        {
            FileInfo fi = new FileInfo(filepath);
            return (fi.Attributes & FileAttributes.Directory) != 0;
        }
    }
}
