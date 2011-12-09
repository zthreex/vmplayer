package com.zthreex.vmplayer.helper;

import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;
import android.util.Log;

public class WifiAdmin {
	private WifiManager mWifiManager;
	private List<ScanResult> mWifiList;
	private List<WifiConfiguration> mWifiConfigurationList;
	WifiLock mWifiLock;
	private static final String VmvideoWifiName = "\"wd-arm11\"";
	//private static final String VmvideoWifiName = "ViewMobile";
	// private static final String VmvideoWifiPassword = "12345678";
	private int vmvideo_server_id = 0;
	private WifiConfiguration wc;

	public WifiAdmin(Context context) {
		mWifiManager = (WifiManager) context
				.getSystemService(Context.WIFI_SERVICE);
		wc = new WifiConfiguration();
		wc.SSID = VmvideoWifiName;
		//wc.preSharedKey = "\"12345678\"";
		wc.status = WifiConfiguration.Status.ENABLED;
		wc.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
		wc.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
		wc.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
		wc.allowedPairwiseCiphers
				.set(WifiConfiguration.PairwiseCipher.TKIP);
		wc.allowedPairwiseCiphers
				.set(WifiConfiguration.PairwiseCipher.CCMP);
		wc.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
	}

	public void OpenWifi() {
		if (!mWifiManager.isWifiEnabled()) {
			mWifiManager.setWifiEnabled(true);
		}
	}

	public void CloseWifi() {
		if (mWifiManager.isWifiEnabled()) {
			mWifiManager.setWifiEnabled(false);
		}
	}

	public void AcquireWifiLock() {
		mWifiLock.acquire();
	}

	public void ReleaseWifiLock() {
		if (mWifiLock.isHeld()) {
			mWifiLock.acquire();
		}
	}

	public void CreatWifiLock() {
		mWifiLock = mWifiManager.createWifiLock("Test");
	}

	public List<WifiConfiguration> GetConfiguration() {
		return mWifiConfigurationList;
	}

	public void StartScan() {
		mWifiManager.startScan();
		mWifiList = mWifiManager.getScanResults();
		mWifiConfigurationList = mWifiManager.getConfiguredNetworks();
	}

	public List<ScanResult> GetWifiList() {
		return mWifiList;
	}

	public StringBuilder LookUpScan() {
		StringBuilder stringBuilder = new StringBuilder();
		for (int i = 0; i < mWifiList.size(); i++) {
			stringBuilder
					.append("Index_" + new Integer(i + 1).toString() + ":");
			stringBuilder.append((mWifiList.get(i)).toString());
			stringBuilder.append("\n");
		}
		return stringBuilder;
	}

	public Boolean ConnectToVmvideo() {
		String probedWifiInfo = "";
		for (int i = 0; i < mWifiList.size(); i++) {
			probedWifiInfo = mWifiList.get(i).toString();
			if (probedWifiInfo.indexOf(VmvideoWifiName) >= 0) {
				for (int j = 0; j < mWifiConfigurationList.size(); j++) {
					if (mWifiConfigurationList.get(j).SSID == VmvideoWifiName) {
						mWifiManager.enableNetwork(mWifiConfigurationList
								.get(j).networkId, true);
					}
				}
			} else {
				vmvideo_server_id = mWifiManager.addNetwork(wc);
				// mWifiManager.updateNetwork(wc);
				// mWifiManager.saveConfiguration();
				mWifiManager.enableNetwork(vmvideo_server_id, true);
				return true;
			}
		}
		return false;
	}
	
	public void ConnectToVmvideo1() {
		vmvideo_server_id = mWifiManager.addNetwork(wc);
		mWifiManager.enableNetwork(vmvideo_server_id, true);
	}

	public void DisableVmvideo() {
		mWifiManager.disconnect();
		mWifiManager.removeNetwork(vmvideo_server_id);
	}

	/* Listen the wifi state */
	public class ConnectionChangeReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			ConnectivityManager mConnectivityManager = (ConnectivityManager) context
					.getSystemService(Context.CONNECTIVITY_SERVICE);
			NetworkInfo ni = mConnectivityManager.getActiveNetworkInfo();
			if (ni.isConnected()) {
				Log.e("my_wifi", "established");
			}
		}
	}
}
