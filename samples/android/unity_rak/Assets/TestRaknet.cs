﻿using Comm.Network;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TestRaknet : MonoBehaviour {

    RakNetClientConnector mCon = null;
    // Use this for initialization
    void Start () {
        mCon = new RakNetClientConnector(1);
        mCon.AddCallback_OnConnected(OnConnect);
        BaseConnector.Log("Start");
    }

    public void OnConnect(object sender, EventArgs e)
    {
        BaseConnector.Log("OnConnect");
        Debug.Log("OnConnect");
    }

    // Update is called once per frame
    void Update () {
		if(mCon!=null)
        {
            mCon.Update();
        }
	}
    void OnGUI()
    {
        if (GUI.Button(new Rect(20, 40, 80, 20), "点这里！"))
        {
            BaseConnector.Log("Connect:");
            //game 战斗服 192.168.85.21  :9822
            mCon.Connect("192.168.85.21", 9822);
            Debug.Log("OK");
        }
    }
}
