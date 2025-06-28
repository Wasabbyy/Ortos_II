# Sound Assets

This folder contains sound effects and music for the game.

## Required Sound Files

The game expects the following WAV format sound files:

### Sound Effects:
- `shoot.wav` - Player shooting sound
- `enemy_hit.wav` - Sound when enemy is hit
- `player_hit.wav` - Sound when player is hit
- `enemy_death.wav` - Sound when enemy dies
- `player_death.wav` - Sound when player dies
- `menu_select.wav` - Menu navigation sound

### Music:
- `background_music.wav` - Background music (loops)

## Audio Format Requirements

- **Format**: WAV (PCM)
- **Sample Rate**: 44100 Hz (recommended)
- **Channels**: Mono or Stereo
- **Bit Depth**: 16-bit (recommended)

## How to Add Your Own Sounds

1. Create your sound files in WAV format
2. Place them in this folder
3. Update the sound loading code in `src/Application.cpp` if you want to use different filenames

## Free Sound Resources

You can find free game sounds at:
- [Freesound.org](https://freesound.org/)
- [OpenGameArt.org](https://opengameart.org/)
- [Zapsplat](https://www.zapsplat.com/) (free with registration)

## Note

The current implementation includes a placeholder WAV loader that generates simple beep sounds. For production use, you should implement a proper WAV file parser or use a library like `stb_vorbis` for OGG files. 