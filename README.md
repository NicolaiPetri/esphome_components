# Nicolai's custom ESPHome components

## Intro
As a big fan of smarthome and especially Home Assistant and now also ESPHome, I have made this repository to shared my ESPHome code that is good enough to be used by others.

## Components

### barchart

A very simple barchart component built from the core "graph" component in ESPHome. 
There is currently lots of missing functionality, but at least it seems stable and as a good starting point.
Be aware this is my first go at a custom ESPHome component :)

One big challenge with ESPHome is that there are currently no support for fetching historical data from Home Assitant using it's integration.
To work around it, first version uses text_sensor where value should be a comma seperated list of values. This can either be managed locally on the esp, or fetched from a Home Asssitant entity. I use the SQL integration in Home Assistant to fetch the values.



## Support me

I do this because I like home automation, but in my "real" life I have created my own startup within cloud and software development.
If you would like to support me then check if any of my products could be useful for you:

### EasyScep - Certificate Authority for Intune

Do you want to use client certificate authentication for your companys WiFi / VPN setups and you are using Microsoft Intune ? 
This is a simple SaaS service that is fast, cheap and reliable.

[EasyScep on Azure Marketplace](https://azuremarketplace.microsoft.com/en-us/marketplace/apps/justsoftwareaps1663178247196.saas-easy-scep)

### EasyRadius - Certificate-based radius authication 

The best companion for EasyRadius, works directly with most radius compatible devices for 802.1x / WPA-Enterprise client certificate authentication.

[EasyRadius on Azure Marketplace](https://azuremarketplace.microsoft.com/en-us/marketplace/apps/justsoftwareaps1663178247196.saas-easy-scep)

### Further information

Feel free to check out our [website](https://just-software.dk) or contant us by [email](mailto:support@just-software.dk)

