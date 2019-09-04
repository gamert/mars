#if USE_BASTHTTP
using System.Collections;
using System.Collections.Generic;
using System;

using BestHTTP;
using BestHTTP.JSON;

namespace Comm.Network
{
    public class HttpHandler : Singleton<HttpHandler>
    {
        public const float kConnectionTimeOut = 10f;

        public delegate object CustomDataHandle(string api, byte[] data);
        public event CustomDataHandle evt_CustomDataHandle;

        public enum HttpTranscationMethods
        {
            GET = 0,
            POST
        }

        public float Timeout = 10f;

        private class SyncTaskContext
        {
            public uint _token = 0;

            public uint token
            {
                get
                {
                    if (_token == 0)
                    {
                        _token = (uint)System.Guid.NewGuid().GetHashCode();
                    }

                    return _token;
                }
            }

            private HTTPRequest httpRequest = null;
            public HTTPRequest RequestObj
            {
                get { return httpRequest; }
            }

            public OnHttpRequestFailureCb failedCallback = null;
            public OnHttpRequestSuccessCb successCallback = null;
            public OnHttpRequestTimeOut timeoutCallback = null;

            private HttpHandler handler = null;
            public SyncTaskContext (HttpHandler owner)
            {
                handler = owner;
            }                     

            public void Request(string url, Hashtable header = null, HttpTranscationMethods method = HttpTranscationMethods.GET)
            {
                if (httpRequest == null)
                {
                    HTTPMethods httpMethods = HTTPMethods.Get;
                    if (method == HttpTranscationMethods.POST)
                        httpMethods = HTTPMethods.Post;

                    httpRequest = new HTTPRequest(new Uri(url), httpMethods, OnHttpRequestFinished);
#if !BESTHTTP_DISABLE_CACHING
                    httpRequest.DisableCache = true;
#endif
                    httpRequest.Timeout = System.TimeSpan.FromSeconds(handler.Timeout);
                    httpRequest.ConnectTimeout = System.TimeSpan.FromSeconds(handler.Timeout);

                    if (header != null)
                    {
                        foreach (DictionaryEntry de in header)
                        {
                            if ((de.Key is string) && (de.Value is string))
                            {
                                httpRequest.AddHeader((string)de.Key, (string)de.Value);
                            }
                        }
                    }

                    httpRequest.Send();
                }
            }

            public void Reset ()
            {
                httpRequest = null;
            }

            private void OnHttpRequestFinished (HTTPRequest request, HTTPResponse response)
            {               
                handler.OnHttpRequestFinished(request, response);
            }
        }

        private List<SyncTaskContext> freeTasks = new List<SyncTaskContext>();
        private List<SyncTaskContext> runningTasks = new List<SyncTaskContext>();

        public delegate void OnHttpRequestFailureCb(int errorCode);
        public delegate void OnHttpRequestSuccessCb(object dataObj);
        public delegate void OnHttpRequestTimeOut();

        private void OnHttpRequestFinished(HTTPRequest request, HTTPResponse response)
        {
            if (request.State == HTTPRequestStates.Finished)
            {
                SyncTaskContext task = FindTask(request);
                task.Reset();
                runningTasks.Remove(task);
                freeTasks.Add(task);
                
                if (response.StatusCode == 200)
                {
                    Dictionary<string, object> data = (Dictionary<string, object>)Json.Decode(request.Response.DataAsText);
                    if (null == data)
                    {
                        if (task.failedCallback != null)
                            task.failedCallback(-1);

                        return;
                    }

                    int error = Convert.ToInt32((double)data["error"]);
                    if (error == 0)
                    {
                        byte[] raw = Convert.FromBase64String((string)data["data"]);
                        object dataObj = CreateDataObj(request.CurrentUri.AbsoluteUri, raw);
                        if (task.successCallback != null)
                            task.successCallback(dataObj);
                    }
                    else
                    {
                        if (task.failedCallback != null)
                            task.failedCallback(error);
                    }
                }
                else
                {
                    if (task.failedCallback != null)
                        task.failedCallback(-1);
                }
            }
            else if (request.State == HTTPRequestStates.ConnectionTimedOut
                || request.State == HTTPRequestStates.TimedOut)
            {
                SyncTaskContext task = FindTask(request);
                task.Reset();
                runningTasks.Remove(task);
                freeTasks.Add(task);

                if (task.timeoutCallback != null)
                    task.timeoutCallback();
            }
            else if (request.State == HTTPRequestStates.Error)
            {
                SyncTaskContext task = FindTask(request);
                task.Reset();
                runningTasks.Remove(task);
                freeTasks.Add(task);

                if (task.failedCallback != null)
                    task.failedCallback(-1);
            }
        }

        private SyncTaskContext FindTask(HTTPRequest request)
        {
            SyncTaskContext task = null;
            foreach (SyncTaskContext ctx in runningTasks)
            {
                if (ctx.RequestObj == request)
                {
                    task = ctx;
                    break;
                }
            }
            return task;
        }

        protected object CreateDataObj(string api, byte[] raw)
        {
            if (evt_CustomDataHandle != null)
                return evt_CustomDataHandle(api, raw);

            return null;
        }

        public uint Request(string url, Hashtable header = null, HttpTranscationMethods method = HttpTranscationMethods.GET, OnHttpRequestSuccessCb succCB = null, OnHttpRequestFailureCb failCB = null, OnHttpRequestTimeOut timeoutCB = null)
        {
            SyncTaskContext task = FetchSyncTask();
            task.successCallback = succCB;
            task.failedCallback = failCB;
            task.timeoutCallback = timeoutCB;
            task.Request(url, header, method);

            return task.token;
        }

        private SyncTaskContext FetchSyncTask()
        {
            SyncTaskContext ctx = null;
            if (freeTasks.Count > 0)
            {
                ctx = freeTasks[0];
                freeTasks.RemoveAt(0);
            }
            else
            {
                ctx = new SyncTaskContext(this);
            }

            runningTasks.Add(ctx);
            return ctx;
        }

        public override void Init()
        {
            HTTPManager.ConnectTimeout = TimeSpan.FromSeconds(kConnectionTimeOut);
        }
    }
}

#endif