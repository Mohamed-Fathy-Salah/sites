## Sites
Sites that I open every day.
## Install
Install `dmenu`
## Usage
### Add new site
`sites [-n -s short -u url [-b before_command] [-a after_command]]`
>switch to first tap in DWM then open leetcode in browser, when the tab is closed open google tasks
 `sites -n -s leetcode -u https://leetcode.com -b 'xdotool key super+1' -a '$BROWSER https://calendar.google.com/calendar/u/1/r/day'`

### Modify site
`sites [-m -i id [-s short] [-u url] [-b before_command] [-a after_command] [-f 0(no)|1(yes)|2(toggle)]]`
>change name to 'new name', toggle finished on site with id = 1 
 `sites -m -i 1 -s 'new name' -f 2`

### Remove site
`sites [-r -i id]`
>remove site with id = 1
 `sites -r -i 1`

### List sites
`sites [-l [-s] [-u] [-b] [-a] [-c] [-f 0(no)|1(yes)]]`
>list name, url of unfinished sites
 `sites -l -su -f 0`

### Open site
`sites`
