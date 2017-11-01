Gem::Specification.new do |s|
  s.name    = "ipcloak"
  s.version = "1.0"
  s.summary = "IP scrambling for ruby developers"
  s.author  = "Christine Dodrill"

  s.files = Dir.glob("ext/**/*.{c,rb}") +
            Dir.glob("lib/**/*.rb")

  s.extensions << "ext/ipcloak/extconf.rb"

  s.add_development_dependency "rake-compiler"
end
