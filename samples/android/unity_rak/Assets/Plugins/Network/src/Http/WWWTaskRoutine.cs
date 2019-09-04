#if USE_BASTHTTP
#define NEW_WEBREQUEST
using UnityEngine;
using System;
//using System.Collections;

#if NEW_WEBREQUEST
//using UnityEngine.Experimental.Networking;
using BestHTTP;
#endif

namespace Comm.Network
{
#if NEW_WEBREQUEST
    [ExecuteInEditMode]
#endif
    public class WWWTaskRoutine : MonoBehaviour
    {
        public delegate void OnDataRetrievedCallback(WWWTaskRoutine task, byte[] data);
        public delegate void OnDataErrorCallback(WWWTaskRoutine task, string error);

        private string _url;
        private OnDataRetrievedCallback _callback = null;
        private OnDataErrorCallback _errorCallback = null;
        private bool _destroyWhenFinish = true;
        private int _timeOutMs = 5000;
        private byte[] _postData = null;
        private HTTPRequest _req = null;

        public string _lastError = null;
        public string LastErr
        {
            get { return _lastError; }
        }

        private bool _isWorking = false;
        // expose to user
        public string url 
        { 
            get { return _url; } 
        }

        public object userdata = null;

#if NEW_WEBREQUEST
        private static byte[] EMPTY_BUFFER = new byte[0]; // to avoid null-reference issue
#endif
        public static WWWTaskRoutine StartTask(string url, OnDataRetrievedCallback cb, OnDataErrorCallback errCb = null, bool destroyWhenFinish = true, int timeOutMs = 3000)
        {
            return StartTask(url, null, cb, errCb, destroyWhenFinish, timeOutMs);
        }

        public static WWWTaskRoutine StartTask(string url, byte[] postData, OnDataRetrievedCallback cb, OnDataErrorCallback errCb = null, bool destroyWhenFinish = true, int timeOutMs = 3000)
        {
            if ( string.IsNullOrEmpty(url) == false )
            {
                GameObject go = new GameObject("_www_task");
                go.hideFlags = HideFlags.HideAndDontSave;
                WWWTaskRoutine task = go.AddComponent<WWWTaskRoutine>();                
                task._callback = cb;
                task._errorCallback = errCb;
                task._url = url;
                task._destroyWhenFinish = destroyWhenFinish;
                task._timeOutMs = timeOutMs;
                task._postData = postData;
                task.StartTask(); 
                return task;
            }
            return null;
        }

        private void StartTask ()
        {
            if ( !_isWorking )
            {
                _isWorking = true;
                if (Application.isPlaying
#if NEW_WEBREQUEST
                    || true
#endif
                    )
                {
#if !NEW_WEBREQUEST
                    StartCoroutine("ProcessRoutine");
#else
                    HTTPMethods callMethod = _postData != null && _postData.Length > 0 ? HTTPMethods.Post : HTTPMethods.Get;
                    HTTPRequest req = new HTTPRequest(new Uri(url + "?p=" + DateTime.UtcNow.Ticks), callMethod, OnHttpRequestFinished);
                    if (callMethod == HTTPMethods.Post)
                        req.AddBinaryData("data", _postData);
                                        
                    req.Timeout = TimeSpan.FromMilliseconds(_timeOutMs);
                    req.ConnectTimeout = TimeSpan.FromMilliseconds(_timeOutMs);
                    req.DisableCache = true;
                    req.Send();
                    _req = req;
                    _isWorking = true;
#endif
                }
                else
                {                    
#if !NEW_WEBREQUEST
                    WWW www = null;
                    if (_postData != null)
                    {
                        WWWForm form = new WWWForm();
                        form.AddBinaryData("data", _postData);
                        www = new WWW(url, form);
                    }
                    else
                        www = new WWW(url);

                    //using (WWW www = new WWW(_url))
                    {
                        long ticks = System.DateTime.Now.Ticks;
                        while (true)
                        {
                            if (www.isDone)
                            {
                                Process(www);
                                break;
                            }

                            long dt = (System.DateTime.Now.Ticks - ticks) / 10000L;
                            if (dt > _timeOutMs)
                            {
                                Debug.LogError(string.Format("WWWTaskRoutine: Request '{0}' Time Out", _url));
                                if (_errorCallback != null)
                                {
                                    _errorCallback(this, "Request Time Out.");
                                }

                                _isWorking = false;
                                if (_destroyWhenFinish)
                                {
                                    if (Application.isPlaying)
                                    {
                                        GameObject.Destroy(gameObject);
                                    }
                                    else
                                    {
                                        GameObject.DestroyImmediate(gameObject);
                                    }
                                }
                                break;
                            }
                        }
                    }
#endif
                }
            }
        }

#if NEW_WEBREQUEST
        void OnHttpRequestFinished(HTTPRequest request, HTTPResponse response)
        {
            bool isFinished = false;
            if (request.State == HTTPRequestStates.Finished)
            {
                if (response.IsSuccess)
                {
                    if (_callback != null)
                        _callback(this, response.Data);
                }
                else
                {
                    if (_errorCallback != null)
                        _errorCallback(this, string.Format("{0}: {1}\r\n{2}", response.StatusCode, response.Message, response.DataAsText));
                }
                isFinished = true;
            }
            else if (request.State == HTTPRequestStates.ConnectionTimedOut
                || request.State == HTTPRequestStates.TimedOut)
            {
                Debug.LogError(string.Format("WWWTaskRoutine: Request '{0}' Time Out", _url));                
                isFinished = true;
                if (_errorCallback != null)
                    _errorCallback(this, "Request time out.");
            }
            else if (request.State == HTTPRequestStates.Error)
            {
                if (request.Exception != null)
                {
                    if (_errorCallback != null)
                        _errorCallback(this, request.Exception.Message);
                }
                else
                {
                    if (_errorCallback != null)
                        _errorCallback(this, "Unknown Failure.");
                }
                isFinished = true;
            }
            else if (request.State == HTTPRequestStates.Aborted)
            {
                Debug.Log(string.Format("WWWTaskRoutine: the request '{0}' has been aborted.", _url));
                isFinished = true;
            }
            
            if (isFinished)
            {
                _isWorking = false;
                if (_destroyWhenFinish)
                {
                    if (Application.isPlaying)
                    {
                        GameObject.Destroy(gameObject);
                    }
                    else
                    {
                        GameObject.DestroyImmediate(gameObject);
                    }
                }
            }        
        }
#endif

#if !NEW_WEBREQUEST
        private void Process(WWW www)
        {            
            if (string.IsNullOrEmpty(www.error))
            {
                if (_callback != null)
                {
                    _callback(this, www.bytes);
                }
            }
            else
            {
                _lastError = www.error;
                if (_errorCallback != null)
                    _errorCallback(this, _lastError);
            }

            _isWorking = false;
            if (_destroyWhenFinish)
            {
                if (Application.isPlaying)
                {
                    GameObject.Destroy(gameObject);
                }
                else
                {
                    GameObject.DestroyImmediate(gameObject);
                }
            }
        }

        private IEnumerator ProcessRoutine ()
        {
            long ticks = System.DateTime.Now.Ticks;

            //using (WWW www = new WWW(_url))
            WWW www = null;
            if (_postData != null)
            {
                WWWForm form = new WWWForm();
                form.AddBinaryData("data", _postData);
                www = new WWW(url, form);
            }
            else
                www = new WWW(url);

            {
                bool timeOut = false;
                do
                {
                    long dt = (System.DateTime.Now.Ticks - ticks) / 10000L;
                    if (dt > _timeOutMs)
                    {
                        timeOut = true;
                        break;
                    }
                    yield return null;
                } while (!www.isDone);

                if (!timeOut)
                {
                    Process(www);
                }
                else
                {
                    Debug.LogError(string.Format("WWWTaskRoutine: Request '{0}' Time Out", _url));
                    if (_errorCallback != null)
                    {
                        _errorCallback(this, "Request Time Out.");
                    }

                    _isWorking = false;
                    if (_destroyWhenFinish)
                    {
                        if (Application.isPlaying)
                        {
                            GameObject.Destroy(gameObject);
                        }
                        else
                        {
                            GameObject.DestroyImmediate(gameObject);
                        }
                    }
                }
            }
        }

        public void Stop ()
        {
            if (_isWorking)
            {
                StopCoroutine(ProcessRoutine());
                _isWorking = false;

                if (_destroyWhenFinish)
                {
                    if (Application.isPlaying)
                    {
                        GameObject.Destroy(gameObject);
                    }
                    else
                    {
                        GameObject.DestroyImmediate(gameObject);
                    }
                }    
            }
        }
#endif

#if NEW_WEBREQUEST
        public void Stop()
        {
            if (_isWorking)
            {
                _req.Abort();
            }
        }
#endif
    }
}
#endif