using RakNet;
using System;
using System.Collections;
using System.Collections.Generic;

namespace Comm.Network
{
    public interface IMonitor
    {
        bool Activate { get; set; }
        float Pulse { get; set; }

        void SetObserver(object ob);
        void Update(float dt);

        void SetData(object key, object data);
        object GetData(object key);
        bool ContainsData(object key);

        void Reset();
        void ForceTick();
    }

    public abstract class BaseMonitor : IMonitor, IDisposable
    {
        protected MessageHandlerBase _handler = null;

        protected bool _activate = false;
        public bool Activate
        {
            get { return _activate; }
            set
            {
                _activate = value;
                OnActivate(value);
            }
        }

        protected float _pulse = 0f;
        public float Pulse
        {
            get { return _pulse; }
            set { _pulse = value; }
        }

        Dictionary<string, ObjectCallback> _listeners = new Dictionary<string, ObjectCallback>();
        Hashtable _dataSlots = new Hashtable();

        private float _ticks = 0f;
        protected void ResetTicks ()
        {
            _ticks = 0f;
        }

        public void SetData (object key, object data)
        {
            if (_dataSlots.ContainsKey(key))
            {
                _dataSlots[key] = data;
            }
            else
            {
                _dataSlots.Add(key, data);
            }
        }

        public object GetData(object key)
        { 
            if (_dataSlots.ContainsKey(key))
            {
                return _dataSlots[key];
            }
            return null;
        }

        public bool ContainsData (object key)
        {
            return _dataSlots.ContainsKey(key);
        }

        public void AddListener(string eventT, ObjectCallback cb)
        {
            if (string.IsNullOrEmpty(eventT) || cb == null)
                return;

            if (_listeners.ContainsKey(eventT))
            {
                _listeners[eventT] = (ObjectCallback)Delegate.Combine(_listeners[eventT], cb);
            }
            else
            {
                _listeners.Add(eventT, cb);
            }
        }

        public void RemoveListener(string eventT, ObjectCallback cb)
        {
            if (_listeners.ContainsKey(eventT))
            {
                _listeners[eventT] = (ObjectCallback)Delegate.Remove(_listeners[eventT], cb);
            }
        }

        public void RemoveAllListeners(string eventT)
        {
            if (_listeners.ContainsKey(eventT))
                _listeners.Remove(eventT);
        }

        public void RemoveAllListeners()
        {
            _listeners.Clear();
        }

        public void Notify(string eventT, object obj = null)
        {
            if (_listeners.ContainsKey(eventT))
            {
                _listeners[eventT](obj);
            }
        }

        abstract public void SetObserver(object ob);
        
        public void Update(float dt)
        {
            if (_activate)
            {
                if (_pulse > 0.001f)
                {
                    _ticks += dt;
                    if (_ticks > _pulse || UnityEngine.Mathf.Abs(_ticks - _pulse) < 0.001f)
                    {
                        _ticks = 0f;
                        OnInterval();
                    }
                }

                OnUpdate(dt);
            }
        }

        public void ForceTick()
        {
            if (_activate)
            {
                _ticks = _pulse;
                Update(UnityEngine.Time.deltaTime);
            }
        }
        
        virtual public void Dispose() { }
        virtual protected void OnInterval() { }
        virtual protected void OnUpdate(float dt) { }
        virtual public void Reset () { }
        virtual protected void OnActivate(bool flag) { }

        public static void Log(object message)
        {
            //_frameLog(message);
            BaseConnector.Log(message);
        }

        protected string GetPrefix()
        {
            return (_handler == null) ? "null" : _handler.conn.GetType().Name;
        }
    }
}