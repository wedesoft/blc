#!/usr/bin/env ruby
M = {
  '.' => '01',
  'P' => '10',
  'Q' => '110',
  'true' => '0000110',
  'false' => '000010'
}
class String
  def forward
    rv = self
    M.each do |k,v|
      rv = rv.gsub k, v
    end
    rv
  end
  def backward
    rv = self
    M.each do |k,v|
      rv = rv.gsub v, k
    end
    rv
  end
end
while !STDIN.eof
  s = STDIN.readline.forward.chomp
  r = `echo #{s} | ./blc`.backward
  puts r
end

