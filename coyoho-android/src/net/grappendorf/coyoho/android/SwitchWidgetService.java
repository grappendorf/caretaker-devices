/*
 * This file is part of the CoYoHo Control Your Home project.
 *
 * Copyright 2011-2012 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package net.grappendorf.coyoho.android;


import java.io.IOException;
import org.apache.http.*;
import org.apache.http.client.*;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.util.EntityUtils;
import android.app.Service;
import android.appwidget.AppWidgetManager;
import android.content.*;
import android.os.IBinder;
import android.util.Log;
import android.widget.RemoteViews;


public class SwitchWidgetService extends Service
{
	private static final String TAG = "SwitchWidgetService";

	@Override
	public void onStart(Intent intent, int startId)
	{
		if (intent.getAction().startsWith("click"))
		{
			widgetClick(intent);
		}

		stopSelf();
	}

	public void widgetClick(Intent intent)
	{
		try
		{
			SharedPreferences prefs = getSharedPreferences(SwitchWidgetConfigurer.PREFS_NAME, 0);
			AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(this.getApplicationContext());
			int appWidgetId = intent.getExtras().getInt("appWidgetId");

			String serverUrl = prefs.getString(SwitchWidgetConfigurer.PREFS_SERVER_URL, "");
			String deviceAddress = prefs.getString(SwitchWidgetConfigurer.PREFS_DEVICE_ADDRESS + appWidgetId, "");
			String switchNumber = prefs.getString(SwitchWidgetConfigurer.PREFS_SWITCH_NUMBER + appWidgetId, "0");

			HttpClient client = new DefaultHttpClient();
			HttpGet httpGet = new HttpGet(serverUrl + "/api/device/" + deviceAddress + "/switch/" + switchNumber
							+ "/toggle");
			HttpResponse response = client.execute(httpGet);
			EntityUtils.toString(response.getEntity());

			httpGet = new HttpGet(serverUrl + "/api/device/" + deviceAddress + "/switch/" + switchNumber);
			response = client.execute(httpGet);
			Boolean state = new Boolean(EntityUtils.toString(response.getEntity()));
			RemoteViews remoteViews = new RemoteViews(this.getApplicationContext().getPackageName(),
							R.layout.switch_widget);
			remoteViews.setInt(
							R.id.switch_widget_layout,
							"setBackgroundResource",
							state.booleanValue() ? SwitchWidgetConfigurer.backgroundResources[prefs.getInt(
											SwitchWidgetConfigurer.PREFS_COLOR + appWidgetId, 0)]
											: R.drawable.widget_grey_shape);
			appWidgetManager.updateAppWidget(appWidgetId, remoteViews);
		}
		catch (ClientProtocolException x)
		{
			Log.e(TAG, x.getMessage());
		}
		catch (IOException x)
		{
			Log.e(TAG, x.getMessage());
		}
		catch (ParseException x)
		{
			Log.e(TAG, x.getMessage());
		}
	}

	@Override
	public IBinder onBind(Intent intent)
	{
		return null;
	}
}
