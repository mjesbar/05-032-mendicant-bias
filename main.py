import os
import ffmpeg
import whisper
from TTS.api import TTS


def stt(name: str = 'voice', fmt: str = 'mp3') -> str:
  NAME: str = name
  FMT: str = fmt
  print("STT: Recording...")
  (
    ffmpeg
    .input('default', f='alsa', ar=48000)
    .output(f'{NAME}.{FMT}', ar=48000, t=3.0)
    .overwrite_output()
    .run(quiet=True)
  )
  print("STT: Recording [OK]")
  # · Processing the mp3 file to text
  print("STT: Processing...")
  model = whisper.load_model('base.en')
  model.cuda()
  ret = model.transcribe(f'{NAME}.{FMT}', temperature=0.8, language='en')
  print("STT: Processing [OK]")
  print("STT: Text:", ret['text'])
  return ret['text']


def ai():
  pass


def tts(text):
  print("TTS: Synthesizing...")
  tts = TTS(model_name="tts_models/en/jenny/jenny",
            progress_bar=False, gpu=True)
  tts.tts_to_file(text=text, file_path='synth.wav')
  print("TTS: Synthesizing [OK]")
  os.system('aplay synth.wav > /dev/null 2>&1')


if __name__ == '__main__':
  text = stt()
  tts(text)
