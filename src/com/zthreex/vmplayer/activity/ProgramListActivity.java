package com.zthreex.vmplayer.activity;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.HashMap;

import com.zthreex.vmplayer.R;
import com.zthreex.vmplayer.helper.WifiAdmin;
import com.zthreex.vmplayer.player.VlcMediaPlayer;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.SimpleAdapter;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

public class ProgramListActivity extends Activity {
	/*Global*/
	String sIpAddress = "http://192.168.0.1/vmvideo/"; //mod1
	private static final int nChannelNum = 12;
	String[] titles = new String[nChannelNum];
	int[] resIds = new int[nChannelNum];

	ProgressDialog mProgressDialog = null;
	WifiAdmin mWifiAdmin;
	EditText mEditTextIPaddress;
	Button mBtnConfirm;
	GridView mGridView;
	
	private ProgressDialog progressDialog = null; 
	static final int MESSAGETYPE_01 = 0x0001;
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		super.onCreateOptionsMenu(menu);
		menu.add(Menu.NONE, 1, 1, "Refresh");
		menu.add(Menu.NONE, 2, 2, "Exit");
		menu.add(Menu.NONE, 3, 3, "Settings");
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case 1:
			titles = sGetProgramList(false);
			if(null == titles){
				Toast.makeText(getApplicationContext(), "Server unreachable", Toast.LENGTH_LONG).show();
			}
			ShowProgramList();
			return true;
		case 2:
			titles = null;
		    //mWifiAdmin.DisableVmvideo();   //mod2
			finish();
			return true;
		case 3:
			/*this.setContentView(R.layout.settings);
			mBtnConfirm = (Button) (findViewById(R.id.btnConfirm));
			mEditTextIPaddress = (EditText) findViewById(R.id.etIPaddress);
			mBtnConfirm.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View arg0) {
					// TODO Auto-generated method stub
					sIpAddress = mEditTextIPaddress.getText().toString();
					setContentView(R.layout.list);
				}
			});
			return true;*/
		}
		return false;
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		this.setContentView(R.layout.gridview);
		this.setTitle("ProgramList");
		
		for(int i=0; i<nChannelNum; i++){
			resIds[i] = R.drawable.cctv1;
		}	
		
		/* Connect to wifi server and acquire the program list */  //mod3
		/*mWifiAdmin = new WifiAdmin(getApplicationContext());
		mWifiAdmin.OpenWifi(); 
		mWifiAdmin.StartScan();
		mWifiAdmin.ConnectToVmvideo1();*/

		int retry_count = 0;
		do {
			titles = sGetProgramList(false);
			retry_count++;
			Log.e("vmvideo", "retry:" + String.valueOf(retry_count));
			if (retry_count >= 5) {
				Toast.makeText(getApplicationContext(), "Server unreachable", Toast.LENGTH_LONG).show();
				break;
			}
		} while (titles[0] == null);
		ShowProgramList();
		mGridView.setOnItemClickListener(new ItemClickListener());
	}
	
	  //当AdapterView被单击(触摸屏或者键盘)，则返回的Item单击事件  
	class ItemClickListener implements OnItemClickListener {
		public void onItemClick(AdapterView<?> arg0,// The AdapterView where the
													// click happened
				View arg1,// The view within the AdapterView that was clicked
				int arg2,// The position of the view in the adapter
				long arg3// The row id of the item that was clicked
		) {
			// 在本例中arg2=arg3
			setTitle("Play" + titles[arg2]);
			StringBuilder uri = new StringBuilder();
			uri.append(sIpAddress);
			
			/*0-5,012B0001 - 012B0006*/
			if( (arg2>=0) && (arg2<=5) )
				uri.append("012b000"+String.valueOf(arg2+1));
			/*6-11,01300001 - 01300006*/
			else
				uri.append("0130000"+String.valueOf(arg2-5));

			Intent intent = new Intent(getApplicationContext(),
					PlayerActivity.class);
			ArrayList<String> playlist = new ArrayList<String>();
			playlist.add(uri.toString());
			intent.putExtra("selected", 0);
			intent.putExtra("playlist", playlist);
			startActivity(intent);
		}
	}
	
	/*Finish activity when Backkey pressed*/
	@Override  
	public void onBackPressed() {   
		this.finish();
	}  

    @Override
    public void onAttachedToWindow() {
    	// TODO Auto-generated method stub
        //this.getWindow().setType(WindowManager.LayoutParams.TYPE_KEYGUARD);
    	super.onAttachedToWindow();
    }

    /*Finish activity when Homekey pressed*/
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
            // TODO Auto-generated method stub
            if(keyCode == KeyEvent.KEYCODE_HOME) {
            	ProgramListActivity.this.finish();
            }
            return super.onKeyDown(keyCode, event);
    }
	
	@Override
	public void onRestart() {
		super.onRestart();
		progressDialog = ProgressDialog.show(ProgramListActivity.this,
				"Exiting", "Removing modules");
		new Thread() {
			public void run() {
				try {
					//Wait for all the modules to be removed
					Boolean status = false;
					do {
						status = VlcMediaPlayer.IsTotallyRemoved();
					} while (status == false);
				} catch (Exception e) {
					// 在GUI显示错误提示
					// tv.setText("Error: " + e.getMessage());
				}
				Message msg_listData = new Message();
				msg_listData.what = MESSAGETYPE_01;
				handler.sendMessage(msg_listData);
			}
		}.start();
	}
  
	private Handler handler = new Handler() {
		public void handleMessage(Message message) {
			switch (message.what) {
			case MESSAGETYPE_01:
				progressDialog.dismiss(); // 关闭进度条
				break;
			}
		}
	};
	
	/*
	 * FIX ME When jump to player activity.remain the wifi connection state
	 */
	@Override
	public void onDestroy() {
		super.onDestroy();
		titles = null;
		//mWifiAdmin.DisableVmvideo();                  //mod4
	}

	@Override
	public void onStop() {
		super.onStop();
	}
	
	/*Make the gridview of programlist*/
	public void ShowProgramList() {
		mGridView =  (GridView) this.findViewById(R.id.gridview);
		ArrayList<HashMap<String, Object>> lstImageItem = new ArrayList<HashMap<String, Object>>();
		for(int i=0; i<nChannelNum; i++){
			HashMap<String, Object> map = new HashMap<String, Object>();
			map.put("ItemImage", resIds[i]);
			map.put("ItemText", titles[i])	;
			lstImageItem.add(map);
		}
		SimpleAdapter saImageItems = new SimpleAdapter(this, 
																									lstImageItem,
																									R.layout.item, 
																									new String[] {"ItemImage", "ItemText"},
																									new int[] {R.id.itemImage, R.id.itemTitle});
		mGridView.setAdapter(saImageItems);
	}

	/*Get the program list.
	 * isAutoAcquire TRUE  Get the program listfrom webserver
	 * isAutoAcquire FALSE Program list has been set earlier
	 * */
	public String[] sGetProgramList(boolean isGetFromServer) {
		String sProgram[] = new String[12];
		if (isGetFromServer) {
			String urlStr = sIpAddress;
			URL url;
			try {
				url = new URL(urlStr);
				URLConnection mURLconnection = url.openConnection();
				HttpURLConnection mhttpConnection = (HttpURLConnection) mURLconnection;
				int responseCode = mhttpConnection.getResponseCode();
				if (responseCode == HttpURLConnection.HTTP_OK) {
					Log.d("vmplayer_wifi", "OK");
					InputStream urlStream = mhttpConnection.getInputStream();
					BufferedReader mbufferedReader = new BufferedReader(
							new InputStreamReader(urlStream));
					String sCurrentLine = "";
					int i = 0;
					while ((sCurrentLine = mbufferedReader.readLine()) != null) {
						int a = sCurrentLine.indexOf("\"");
						int b = sCurrentLine.indexOf("\"", 14);
						sProgram[i++] = sCurrentLine.substring(a + 1, b);
					}
					Log.d("vmvideo_wifi", sProgram[0]);
				} else {
					Log.d("vmvideo_wifi", "FAILED");
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		else{
			sProgram[0] = "CCTV-NEWS";
			sProgram[1] = "CCTV-KIDS";
			sProgram[2] = "CCTV-8";
			sProgram[3] = "CCTV-5";
			sProgram[4] = "CCTV-2";
			sProgram[5] = "CCTV-11";
			sProgram[6] = "CCTV-NEWS";
			sProgram[7] = "CCTV-KIDS";
			sProgram[8] = "CCTV-8";
			sProgram[9] = "CCTV-5";
			sProgram[10] = "CCTV-2";
			sProgram[11] = "CCTV-11";
		}
		return sProgram;
	}

}