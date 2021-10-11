using Microsoft.VisualStudio.Shell;
using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Threading;
using Task = System.Threading.Tasks.Task;

namespace G6IdlToCpp
{
    /// <summary>
    /// This is the class that implements the package exposed by this assembly.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The minimum requirement for a class to be considered a valid package for Visual Studio
    /// is to implement the IVsPackage interface and register itself with the shell.
    /// This package uses the helper classes defined inside the Managed Package Framework (MPF)
    /// to do it: it derives from the Package class that provides the implementation of the
    /// IVsPackage interface and uses the registration attributes defined in the framework to
    /// register itself and its components with the shell. These attributes tell the pkgdef creation
    /// utility what data to put into .pkgdef file.
    /// </para>
    /// <para>
    /// To get loaded into VS, the package must be referred by &lt;Asset Type="Microsoft.VisualStudio.VsPackage" ...&gt; in .vsixmanifest file.
    /// </para>
    /// </remarks>
    [PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
    [Guid(G6IdlToCppPackage.PackageGuidString)]
    [ProvideMenuResource("Menus.ctmenu", 1)]
    [InstalledProductRegistration("#110", "#112", "1.0", IconResourceID = 400)]
    [ProvideMenuResource("Menus.ctmenu", 1)]
    [ProvideOptionPage(typeof(IdlToCppOptionPage), "IDL To Cpp", "General", 0, 0, true)]

    public sealed class G6IdlToCppPackage : AsyncPackage
    {
        /// <summary>
        /// G6IdlToCppPackage GUID string.
        /// </summary>
        public const string PackageGuidString = "4f4ef72b-2ca2-46bb-a513-5303543d623c";

        #region Package Members

        /// <summary>
        /// Initialization of the package; this method is called right after the package is sited, so this is the place
        /// where you can put all the initialization code that rely on services provided by VisualStudio.
        /// </summary>
        /// <param name="cancellationToken">A cancellation token to monitor for initialization cancellation, which can occur when VS is shutting down.</param>
        /// <param name="progress">A provider for progress updates.</param>
        /// <returns>A task representing the async work of package initialization, or an already completed task if there is none. Do not return null from this method.</returns>
        protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress)
        {
            // When initialized asynchronously, the current thread may be a background thread at this point.
            // Do any initialization that requires the UI thread after switching to the UI thread.
            await this.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
            await idlToCpp.InitializeAsync(this);
        }

        #endregion

        public string OptionGeneratorPath
        {
            get
            {
                IdlToCppOptionPage page = GetDialogPage(typeof(IdlToCppOptionPage)) as IdlToCppOptionPage;
                return page.OptionGeneratorPath;
            }
        }

        public string OptionOutputPath
        {
            get
            {
                IdlToCppOptionPage page = GetDialogPage(typeof(IdlToCppOptionPage)) as IdlToCppOptionPage;
                return page.OptionOutputPath;
            }
        }
    }

    public class IdlToCppOptionPage : DialogPage
    {
        [Category("生成器")]
        [DisplayName("生成器路径")]
        [Description("自定义IDL的代码生成器（interface_generator_app.exe）路径.")]
        public string OptionGeneratorPath { get; set; }

        [Category("输出")]
        [DisplayName("输出路径")]
        [Description("生成的C++代码存放路径.")]
        [DefaultValue("idl-out")]
        public string OptionOutputPath { get; set; } = "idl-out";
    }
}
