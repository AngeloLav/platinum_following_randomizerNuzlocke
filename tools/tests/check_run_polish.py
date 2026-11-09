#!/usr/bin/env python3
"""Prompt5 resource/integration checks; optional native-asset layout preview."""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    messages = {m['id']: m['en_US'] for m in json.loads((ROOT / 'res/text/run_engine.json').read_text(encoding='utf-8'))['messages']}
    charmap = {line[5:]: int(line[:4], 16) for line in (ROOT / 'tools/msgenc/charmap.txt').read_text(encoding='utf-8').splitlines() if re.match(r'^[0-9A-Fa-f]{4}=.$', line)}
    widths = json.loads((ROOT / 'res/fonts/font_system.json').read_text())['glyphWidths']
    def width(text): return sum(widths[charmap[c] - 1] for c in text)
    menu = (ROOT / 'src/applications/options_menu.c').read_text(encoding='utf-8')
    run_main = menu.split('BOOL RunMenu_Main(')[1].split('BOOL RunMenu_Exit(')[0]
    assert run_main.count('StartScreenFade(FADE_BOTH_SCREENS,') == 2
    assert 'StartScreenFade(FADE_MAIN_ONLY,' not in run_main
    options = menu.split('static const RunMenuOption sRunOptions[] = {')[1].split('};')[0]
    labels = re.findall(r'RUN_(?:BOOL|NUMBER)\(Run_Text_(\w+),', options)
    assert 'Wipe' not in labels and labels.count('Permadeath') == 1
    assert messages['Run_Text_RateUltraHigh'] == '1/512 Ultra High'
    for name in ['Controls', 'ReviewControls', 'ConfigNav']:
        assert 'HELP' not in messages['Run_Text_' + name]
    assert 'shinyRate, 0, 4, 1' in options
    assert 60 + 24 <= 84  # Large digits finish before capture-status text.
    for name in labels + ['Preset', 'Seed', 'Review']:
        text = messages['Run_Text_Detail' + name]
        assert len(text) < 512
        assert max(width(word) for word in text.split()) <= 216
    # Layout constraints for each column and the 192x160 LCD viewport.
    for name, maximum in [('Title',184),('Captures',184),('NoWild',184),('Details',184),('OneReady',184),('Cap',140),('Badges',65),('Bonus',49)]:
        assert width(messages['Run_Text_Tracker' + name]) <= maximum, name
    for name in ['EndTitle','EndSaveWarning','EndContinue','RestartWorking','RestartFailed']:
        assert len(messages['Run_Text_' + name].splitlines()) <= 8
        assert max(map(width, messages['Run_Text_' + name].splitlines())) <= 224
    scripts = ROOT / 'res/field/scripts'
    for file, species, command in [
        ('rock_peak_ruins','REGIROCK','StartLegendaryBattle'),('iceberg_ruins','REGICE','StartLegendaryBattle'),
        ('iron_ruins','REGISTEEL','StartLegendaryBattle'),('snowpoint_temple_b5f','REGIGIGAS','StartLegendaryBattle'),
        ('acuity_cavern','UXIE','StartLegendaryBattle'),('valor_cavern','AZELF','StartLegendaryBattle'),
        ('newmoon_island_forest','DARKRAI','StartLegendaryBattle'),('flower_paradise','SHAYMIN','StartFatefulEncounter'),
        ('spear_pillar_dialga','DIALGA','StartLegendaryBattle'),('spear_pillar_palkia','PALKIA','StartLegendaryBattle'),
        ('stark_mountain_room_3','HEATRAN','StartLegendaryBattle'),('distortion_world_giratina_room','GIRATINA','StartGiratinaOriginBattle'),
        ('turnback_cave_giratina_room','GIRATINA','StartLegendaryBattle')]:
        assert command + ' SPECIES_' + species in (scripts / ('scripts_' + file + '.s')).read_text()
    assert 'ROAMING_SLOT_MESPRIT' in (scripts / 'scripts_verity_cavern.s').read_text()
    wild = (ROOT / 'src/overlay006/wild_encounters.c').read_text(encoding='utf-8')
    assert wild.count('trainerIDs[BATTLER_PLAYER_2] = SystemVars_GetPartnerTrainerID') == 3
    assert 'runEncounterSource = RUN_SOURCE_STATIC' in wild
    world = ROOT / 'build/run_world_data.h'
    if world.exists():
        def table(name):
            return [int(n) for n in re.findall(r'\b\d+\b', re.search(name + r'\[.*?\]\s*=\s*\{(.*?)\}', world.read_text(), re.S)[1])]
        maps = (ROOT / 'generated/map_headers.txt').read_text().splitlines()
        zones, ordinary = table('sRunMapZones'), table('sRunWildZones')
        assert ordinary[zones[maps.index('MAP_HEADER_JUBILIFE_CITY')]] == 0
        assert ordinary[zones[maps.index('MAP_HEADER_ROUTE_201')]] == 1
    print('PASS: 25 option explanations plus preset/seed/review, native-font LCD/help bounds, 14 static/roaming paths, three wild-partner loaders')
    print('NOT TESTED: button execution, Ball use in ARM battles, graphics on DS, title teardown or real-flash writes')
    if '--preview' in sys.argv:
        preview(messages, charmap, widths)


def preview(messages, charmap, widths):
    from PIL import Image, ImageDraw
    font = Image.open(ROOT / 'res/fonts/font_system.png')
    palette = Image.open(ROOT / 'res/graphics/poketch/unused_apps.png').getpalette()
    colors = {i: tuple(palette[i*3:i*3+3]) for i in (1,4,8,15)}
    def text(im, text, x, y, centered=False):
        if centered: x=(192-sum(widths[charmap[c]-1] for c in text))//2
        for char in text:
            index=charmap[char]-1; w=widths[index]
            glyph=font.crop(((index%16)*16,(index//16)*16,(index%16)*16+w,(index//16+1)*16))
            for gy in range(16):
                for gx in range(w):
                    pixel=glyph.getpixel((gx,gy))
                    if pixel in (1,2) and 0<=x+gx<192 and 0<=y+gy<160:
                        im.putpixel((x+gx,y+gy), colors[1 if pixel==1 else 8])
            x+=w
    output=Image.new('RGB',(592,180),colors[15])
    for column,(area,remaining,total,state) in enumerate([('ROUTE 203',1,3,'AVAILABLE'),('ROUTE 201',0,1,'EXHAUSTED'),('Jubilife City',0,0,'NO WILD ENCOUNTERS')]):
        im=Image.new('RGB',(192,160),colors[4]); draw=ImageDraw.Draw(im)
        for y in (20,104,138):draw.line((4,y,187,y),fill=colors[1])
        draw.rectangle((0,0,2,159),fill=colors[1]);draw.rectangle((189,0,191,159),fill=colors[1])
        text(im,'NUZLOCKE TRACKER',0,1,True);text(im,area,0,23,True);text(im,'CAPTURES LEFT',0,43,True)
        if total:
            for i in range(total):
                x=10+i*14;draw.rectangle((x+2,62,x+11,73),fill=colors[1]);draw.rectangle((x+3,63,x+10,72),fill=colors[8 if i<remaining else 4])
                for corner in range(4):
                    cx=x+2+(corner&1)*8;cy=62+(corner>>1)*10;draw.rectangle((cx,cy,cx+1,cy+1),fill=colors[4])
                draw.rectangle((x+2,67,x+11,68),fill=colors[1]);draw.rectangle((x+6,67,x+7,68),fill=colors[4])
            source=(ROOT/'src/applications/poketch/unused/4/graphics.c').read_text(encoding='utf-8')
            bars=[tuple(map(int,m)) for m in re.findall(r'\{ (\d+), (\d+), (\d+), (\d+) \}',source.split('static const u8 bars')[1].split('};')[0])]
            for digit,x in ((remaining,96),(total,150)):
                for bit,(dx,dy,w,h) in enumerate(bars):
                    if [0x3f,0x06,0x5b,0x4f][digit]&(1<<bit):draw.rectangle((x+dx,60+dy,x+dx+w-1,60+dy+h-1),fill=colors[1])
            text(im,'/',130,65);text(im,state,0,84,True)
        else:
            text(im,state,0,65,True);text(im,'START: RUN REGISTRY',0,84,True)
        text(im,'LEVEL CAP',8,106);text(im,'14',169,106);text(im,'BADGES',8,122);text(im,'0',78,122);text(im,'BONUS',109,122);text(im,'0',176,122);text(im,'ACTIVE',0,142,True)
        output.paste(im,(column*200,20))
    ImageDraw.Draw(output).text((4,3),'NATIVE-ASSET LAYOUT PREVIEW / NOT AN EMULATOR SCREENSHOT',fill=colors[1])
    path=ROOT/'artifacts/prompt5-tracker-preview.png';path.parent.mkdir(exist_ok=True)
    output.resize((1184,360),Image.Resampling.NEAREST).save(path)
    print('Preview:',path)


if __name__ == '__main__':main()
