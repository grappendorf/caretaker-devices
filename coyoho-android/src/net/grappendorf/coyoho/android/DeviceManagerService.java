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


import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;


public class DeviceManagerService extends Service
{
	private Thread updateThread;

	private boolean running = true;

	@Override
	public IBinder onBind(Intent arg0)
	{
		return null;
	}

	@Override
	public void onCreate()
	{
		super.onCreate();
		updateThread = new Thread()
		{
			@Override
			public void run()
			{
				while (running)
				{
					Log.e("###", "§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§");
					try
					{
						sleep(10000);
					}
					catch (InterruptedException ignored)
					{
					}
					Log.e("###", "end");
				}
			}
		};
		updateThread.start();
	}

	@Override
	public void onDestroy()
	{
		Log.e("###", "destroy");
		running = false;
		try
		{
			updateThread.join();
		}
		catch (InterruptedException ignored)
		{
		}
		Log.e("###", "whooo");
		super.onDestroy();
	}
}
