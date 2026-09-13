using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Diagnostics;
using System.Threading;


namespace rc_client_portable
{
    class ServerState
    {
        public HttpListener Listener;
        public string Root;
    }


    class Program
    {
        static void Main(string[] args)
        {
            bool createdNew;

            // Create a named mutex that lives for the lifetime of the process
            using (var mutex = new Mutex(true, "Global\\RC_CLIENT_SINGLE_INSTANCE", out createdNew))
            {
                if (!createdNew)
                {
                    Console.WriteLine("Another instance is already running... Exiting.");
                    Thread.Sleep(2000);
                    return; // deny second instance
                }

                // Continue startup...
                Console.WriteLine("Starting AMTCS remote control...");
                RunRemoteControlClient();
            }
        }

        static void RunRemoteControlClient() { 
            string extractRoot = @"C:\rc_client_portable";
            Directory.CreateDirectory(extractRoot);

            var asm = Assembly.GetExecutingAssembly();
            var resourceNames = asm.GetManifestResourceNames();

            foreach (var name in asm.GetManifestResourceNames())
            {
                Console.WriteLine("RESOURCE: " + name);
            }


            foreach (var fullName in resourceNames)
            {
                // Strip namespace prefix
                string name = fullName.Substring(fullName.IndexOf('.') + 1);

                // Convert dots to slashes
                name = name.Replace('.', '/');

                // Fix extension (last slash becomes dot)
                int lastSlash = name.LastIndexOf('/');
                name = name.Substring(0, lastSlash) + "." + name.Substring(lastSlash + 1);

                string outputPath = Path.Combine(extractRoot, name);

                // Create directory structure
                string dir = Path.GetDirectoryName(outputPath);
                Directory.CreateDirectory(dir);

                using (var stream = asm.GetManifestResourceStream(fullName))
                using (var file = File.Create(outputPath))
                {
                    stream.CopyTo(file);
                }
            }

            // Start HTTP server
            int port = 8000;
            string prefix = $"http://localhost:{port}/";

            HttpListener listener = new HttpListener();
            listener.Prefixes.Add(prefix);

            try
            {
                listener.Start();
            }
            catch (HttpListenerException ex)
            {
                Console.WriteLine("ERROR: Cannot bind to port 8000.");
                Console.WriteLine("Reason: " + ex.Message);
                Thread.Sleep(2000);
                return;
            }

            var state = new ServerState
            {
                Listener = listener,
                Root = extractRoot
            };

            listener.BeginGetContext(OnRequest, state);


            Console.WriteLine("Serving client at " + prefix);

            // Launch browser
            System.Diagnostics.Process.Start(prefix);

            Console.WriteLine("Press Enter to exit...");
            Console.ReadLine();

            listener.Stop();
            listener.Close();
        }

        static void OnRequest(IAsyncResult ar)
        {
            var state = (ServerState)ar.AsyncState;
            var listener = state.Listener;
            var root = state.Root;

            HttpListenerContext ctx = null;

            // Safely complete the async request
            try
            {
                ctx = listener.EndGetContext(ar);
            }
            catch (HttpListenerException)
            {
                // Listener was stopped — normal shutdown
                return;
            }
            catch (ObjectDisposedException)
            {
                // Listener was disposed — normal shutdown
                return;
            }

            // Queue next request safely
            try
            {
                listener.BeginGetContext(OnRequest, state);
            }
            catch
            {
                // Listener is shutting down — ignore
                return;
            }

            // Your original logic continues unchanged
            string urlPath = ctx.Request.Url.AbsolutePath.TrimStart('/');
            if (urlPath == "")
                urlPath = "index.html";

            string filePath = Path.Combine(root, urlPath);

            if (!File.Exists(filePath))
            {
                ctx.Response.StatusCode = 404;
                ctx.Response.Close();
                return;
            }

            ctx.Response.ContentType = GetMimeType(filePath);

            using (FileStream fs = File.OpenRead(filePath))
            {
                fs.CopyTo(ctx.Response.OutputStream);
            }

            ctx.Response.OutputStream.Close();
        }

        static string GetMimeType(string path)
        {
            string ext = Path.GetExtension(path).ToLower();
            if (ext == ".html") return "text/html";
            if (ext == ".js") return "application/javascript";
            if (ext == ".css") return "text/css";
            if (ext == ".png") return "image/png";
            if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
            return "application/octet-stream";
        }

    }
}
