package com.zthreex.vmplayer.activity;

import android.app.Activity;
import android.os.Bundle;

import java.util.ArrayList;

import com.zthreex.vmplayer.R;

import android.content.Intent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class TestActivity extends Activity {
	/** Called when the activity is first created. */

	EditText mTextUri = null;
	Button mButtonPlay = null;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.test);
		mTextUri = (EditText) findViewById(R.id.test_uri);
		mButtonPlay = (Button) findViewById(R.id.test_play);
		mButtonPlay.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View view) {
				String uri = mTextUri.getText().toString();
				if (uri.length() > 0) {
					Intent intent = new Intent(TestActivity.this,
							PlayerActivity.class);
					ArrayList<String> playlist = new ArrayList<String>();
					playlist.add(uri);
					intent.putExtra("selected", 0);
					intent.putExtra("playlist", playlist);
					startActivity(intent);
				}
			}
		});
	}

}