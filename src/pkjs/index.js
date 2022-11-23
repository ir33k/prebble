var Clay = require('pebble-clay')

var vibs = [
  { "value": 0, "label": "None" },
  { "value": 1, "label": "Short" },
  { "value": 2, "label": "Long" },
  { "value": 3, "label": "Double" }
]

new Clay([
  {
    "type": "heading",
    "defaultValue": "pRebble"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Background"
      },
      {
        "type": "select",
        "messageKey": "BGTYPE",
        "label": "Type",
        "defaultValue": 1,
        "options": [
          { "value": 0, "label": "None" },
          { "value": 1, "label": "Solid color" },
          { "value": 2, "label": "Battery charge" }
        ]
      },
      {
        "type": "color",
        "messageKey": "BGCOLOR",
        "label": "Color",
        "allowGray": true
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Background pattern"
      },
      {
        "type": "select",
        "messageKey": "FGTYPE",
        "label": "Type",
        "defaultValue": 1,
        "options": [
          { "value": 0, "label": "None" },
          { "value": 1, "label": "Diagonal lines" },
          { "value": 2, "label": "Dots" }
        ]
      },
      {
        "type": "color",
        "messageKey": "FGCOLOR",
        "label": "Color",
        "allowGray": true
      },
      {
        "type": "toggle",
        "messageKey": "FGBT",
        "label": "Hide on Bluetooth disconnect",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Vibrations"
      },
      {
        "type": "select",
        "messageKey": "VIBEBT",
        "label": "Bluetooth connection",
        "defaultValue": 0,
        "options": vibs
      },
      {
        "type": "select",
        "messageKey": "VIBEH",
        "label": "Hourly",
        "defaultValue": 0,
        "options": vibs
      },
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Other"
      },
      {
        "type": "select",
        "messageKey": "DATE",
        "label": "Date format",
        "defaultValue": 1,
        "options": [
          { "value": "",         "label": "Default" },
          { "value": "%A %m.%d", "label": "Sunday 11.18" },
          { "value": "%a %m.%d", "label": "Su 11.18" },
          { "value": "%B %d",    "label": "November 18" },
          { "value": "%b %d",    "label": "Nov 18" },
          { "value": "%A %d",    "label": "Sunday 18" },
          { "value": "%a %d",    "label": "Su 18" },
          { "value": "%Y.%m.%d", "label": "2022.11.18" },
          { "value": "%d.%m.%Y", "label": "18.11.2022" },
          { "value": "%m.%d.%Y", "label": "11.18.2022" }
        ]
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
], function () {
  // Disable DEST item when SRC item value is one of VALUES.
  function disableOnChange(src, dest, values) {
    function handle() {
      dest[values.indexOf(src.get()) === -1 ? "enable" : "disable"]()
    }
    handle()
    src.on('change', handle)
  }

  this.on(this.EVENTS.AFTER_BUILD, function () {
    disableOnChange(
      this.getItemByMessageKey('BGTYPE'),
      this.getItemByMessageKey('BGCOLOR'),
      ["0", "2"]
    )
    disableOnChange(
      this.getItemByMessageKey('FGTYPE'),
      this.getItemByMessageKey('FGCOLOR'),
      ["0"]
    )
    disableOnChange(
      this.getItemByMessageKey('FGTYPE'),
      this.getItemByMessageKey('FGBT'),
      ["0"]
    )
  }.bind(this))
})
