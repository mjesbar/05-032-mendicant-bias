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
  print("STT: Recorded [OK]")
  # · Processing the mp3 file to text
  print("STT: Processing...")
  model = whisper.load_model('tiny.en')
  model.cpu()
  ret = model.transcribe(f'{NAME}.{FMT}', temperature=1.0, language='en')
  print("STT: Processed [OK]")
  print("STT: ReadText:", ret['text'])
  return ret['text']


def ai():
  pass


def tts(text):
  print("TTS: Synthesizing...")
  tts = (
    TTS(model_name="tts_models/en/jenny/jenny", progress_bar=False)
    .to('cuda')
  )
  tts.tts_to_file(text=text, file_path='synth.wav')
  print("TTS: Synthesized [OK]")
  print("TTS: Playing...")
  os.system('aplay synth.wav > /dev/null 2>&1')
  print("TTS: Played [OK]")


def main():
  text = stt()
  tts(text)


if __name__ == '__main__':
  main()
