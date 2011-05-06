class Fixnum
  def power(other)
    (self - other) ** 2
  end
end

class Array
  def power(other)
    self.zip(other).map do |pair|
      (pair.first - pair.last) ** 2
    end
  end
end

class Link
  def initialize(from, to, power)
    @from = from
    @to = to
    @power = power
  end

  def from
    @from
  end

  def to
    @to
  end

  def power
    @power
  end

  def to_s
    "| #{from} - #{to}: #{power} |"
  end
end

def greedy_exchange(a, b)
  ax = 0
  powermatrix = a.map do |from|
    bx = 0
    links = b.map do |to|
      link = Link.new(ax, bx, from.power(to))
      bx += 1
      link
    end.sort_by {|link| link.power}
    ax += 1
    links
  end.group_by {|links| links.first.to}

  # keep track of what is matched from before and after separately
  bunmatched = (0...b.size).reject do |x|
    powermatrix[x]
  end
  aunmatched = []

  while overlap = powermatrix.keys.detect {|key| powermatrix[key].size > 1}
    if alone = bunmatched.first
      weakest = powermatrix[overlap].min do |pre, post|
        pre.detect {|link| link.to == alone}.power <=> post.detect {|link| link.to == alone}.power
      end

      powermatrix[overlap].delete(weakest)

      # rearrange the chosen best unmatched in b to come first in the list, 
      # pushing the previous leader to the end.
      head = weakest.first
      weakest.push(head)
      strong = weakest.detect {|link| link.to == alone}
      weakest.delete(strong)
      weakest[0] = strong

      powermatrix[alone] = [weakest]
      bunmatched.shift
    else
      weakest = powermatrix[overlap].max do |pre, post|
        pre.first.power <=> post.first.power
      end
      powermatrix[overlap].delete(weakest)
      aunmatched.push(weakest)
    end
  end

  links = powermatrix.keys.map do |key|
    powermatrix[key].first.first
  end

  [links, aunmatched, bunmatched]
end

# links, aun, bun = greedy_exchange([5, 14, 200, 99], [8, 30, 40])
links, aun, bun = greedy_exchange([[ 188, 455 ],
                                   [ 54, 448 ],
                                   [ 306, 158 ],
                                   [ 97, 18 ]],
                                  [[ 193, 455 ],
                                   [ 47, 445 ],
                                   [ 306, 164 ]])

puts "links - #{links}"
puts "unmatched A - #{aun.inspect}"
puts "unmatched B - #{bun.inspect}"
