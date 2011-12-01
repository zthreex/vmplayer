package com.zthreex.vmplayer.activity;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;

import com.zthreex.vmplayer.R;
import com.zthreex.vmplayer.helper.WifiAdmin;
import com.zthreex.vmplayer.player.VlcMediaPlayer;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

public class ProgramListActivity extends Activity {
	/*Global*/
	WifiAdmin mWifiAdmin;
	ListView listView;
	String[] titles = new String[12];
	int[] resIds = { R.drawable.cctv1, R.drawable.cctv1, R.drawable.cctv1,
			R.drawable.cctv1, R.drawable.cctv1, R.drawable.cctv1,
			R.drawable.cctv1, R.drawable.cctv1, R.drawable.cctv1,
			R.drawable.cctv1, R.drawable.cctv1, R.drawable.cctv1 };
	
	ProgressDialog mProgressDialog = null;
	ConnectivityManager cm;
	NetworkInfo netinfo;
	String sIpAddress = "http://192.168.0.1/vmvideo/"; //mod1
	EditText mEditTextIPaddress;
	Button mBtnConfirm;
	
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
			titles = sGetProgramList(true);
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
		this.setContentView(R.layout.list);
		this.setTitle("ProgramList");
		ShowProgramList();
		
		/* Connect to wifi server and acquire the program list */  //mod3
		/*mWifiAdmin = new WifiAdmin(getApplicationContext());
		mWifiAdmin.OpenWifi(); 
		mWifiAdmin.StartScan();
		mWifiAdmin.ConnectToVmvideo1();*/

		int retry_count = 0;
		do {
			//titles = sGetProgramList(false);
			titles = sGetProgramList(true);
			retry_count++;
			Log.e("vmvideo", "retry:" + String.valueOf(retry_count));
			if (retry_count >= 5) {
				Toast.makeText(getApplicationContext(), "Server unreachable", Toast.LENGTH_LONG).show();
				break;
			}
		} while (titles[0] == null);
		ShowProgramList();

		listView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				// TODO Auto-generated method stub
				setTitle("Play" + titles[arg2]);
				StringBuilder uri = new StringBuilder();
				uri.append(sIpAddress);
				switch (arg2) {
				case 0:
					uri.append("012b0001");
					break;
				case 1:
					uri.append("012b0002");
					break;
				case 2:
					uri.append("012b0003");
					break;
				case 3:
					uri.append("012b0004");
					break;
				case 4:
					uri.append("012b0005");
					break;
				case 5:
					uri.append("012b0006");
					break;
				case 6:
					uri.append("01300001");
					break;
				case 7:
					uri.append("01300002");
					break;
				case 8:
					uri.append("01300003");
					break;
				case 9:
					uri.append("01300004");
					break;
				case 10:
					uri.append("01300005");
					break;
				case 11:
					uri.append("01300006");
					break;
				default:
					break;
				}
				Intent intent = new Intent(getApplicationContext(),
						PlayerActivity.class);
				ArrayList<String> playlist = new ArrayList<String>();
				playlist.add(uri.toString());
				intent.putExtra("selected", 0);
				intent.putExtra("playlist", playlist);
				startActivity(intent);
			}
		});
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

	public void ShowProgramList() {
		listView = (ListView) this.findViewById(R.id.mListView);
		listView.setAdapter(new ListViewAdapter(titles, resIds));
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

	/* Icon lists class */
	public class ListViewAdapter extends BaseAdapter {
		View[] itemViews;

		public ListViewAdapter(String[] itemTitles, int[] itemImageRes) {
			itemViews = new View[itemTitles.length];
			for (int i = 0; i < itemViews.length; i++) {
				itemViews[i] = makeItemView(itemTitles[i], itemImageRes[i]);
			}
		}

		public int getCount() {
			return itemViews.length;
		}

		public View getItem(int position) {
			return itemViews[position];
		}

		public long getItemId(int position) {
			return position;
		}

		private View makeItemView(String strTitle, int resId) {
			LayoutInflater inflater = (LayoutInflater) ProgramListActivity.this
					.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

			View itemView = inflater.inflate(R.layout.item, null);

			TextView title = (TextView) itemView.findViewById(R.id.itemTitle);
			title.setText(strTitle);
			ImageView image = (ImageView) itemView.findViewById(R.id.itemImage);
			image.setImageResource(resId);

			return itemView;
		}

		public View getView(int position, View convertView, ViewGroup parent) {
			if (convertView == null)
				return itemViews[position];
			return convertView;
		}
	}

}