package com.zx.androidffmpegrecoder.songstudio.recording.service.factory;


import com.zx.androidffmpegrecoder.songstudio.recording.RecordingImplType;
import com.zx.androidffmpegrecoder.songstudio.recording.service.RecorderService;
import com.zx.androidffmpegrecoder.songstudio.recording.service.impl.AudioRecordRecorderServiceImpl;

public class RecorderServiceFactory {
	private static RecorderServiceFactory instance = new RecorderServiceFactory();
	private RecorderServiceFactory(){};
	public static RecorderServiceFactory getInstance(){
		return instance;
	}
	
	public RecorderService getRecorderService(RecordingImplType recordingImplType){
		RecorderService result = null;
		switch (recordingImplType) {
		case ANDROID_PLATFORM:
			result = AudioRecordRecorderServiceImpl.getInstance();
			break;
		default:
			break;
		}
		return result;
	}
}
