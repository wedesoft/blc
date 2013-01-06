#!/usr/bin/env ruby
M = {
  '.' => '01',
  'P' => '111110',
  'Q' => '1111110',
  'R' => '11111110',
  'true' => '0000110',
  'false' => '000010',
  'if' => '0101',
  'null' => '000010',
  'cons' => '00010110',
  'first' => '0000 110',
  'rest' => '0000 10'
}
# cons 1P 1Q
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

