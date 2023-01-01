## Sites
Sites that I open every day.

## Install
Install `dmenu`, `herbe`
Run `sudo make`
Add environment variable `SITES="firefox"` (replace firefox with watever browser u use)

## Usage
### Add new site
`sites -n -s short -u url [-b before_command] [-a after_command]`
>Switch to first tap in DWM then open leetcode in browser, when the tab is closed open google tasks
 `sites -n -s leetcode -u https://leetcode.com -b 'xdotool key super+1' -a '$BROWSER https://calendar.google.com/calendar/u/1/r/day'`

### Modify site
`sites -m -i id [-s short] [-u url] [-b before_command] [-a after_command] [-f 0(no)|1(yes)|2(toggle)]`
>Change name to 'new name', toggle finished on site with id = 1 
 `sites -m -i 1 -s 'new name' -f 2`

### Remove site
`sites -r -i id`
>Remove site with id = 1
 `sites -r -i 1`

### List sites
`sites -l [-s] [-u] [-b] [-a] [-c] [-f 0(no)|1(yes)]`
>List name, url of unfinished sites
 `sites -l -su -f 0`

### Execute site
`sites -e -i id [-f update finished to true]`
>Run site of `id=4` using `$SITES` and its state to finished
 `sites -e -i 4 -f`

### Open site
`sites`
>Open in dmenu unfinished sites, select one of them, site will open in `$SITES`

### Print help
`sites -h`
