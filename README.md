# ipcloak

This provides IP cloaking or scrambling tools for making it easier to allow bans
by IP address made by moderators you don't trust with said IP addresses.

Usage:

```ruby
require "ipcloak"

IPCloak::ip("8.8.8.8")
# => "8.8.l.u"

IPCloak::host("some-customer-subdomain.localisp.tld")
# => "ppan-bethxlos-byvyqcnpn.localisp.tld"
```
