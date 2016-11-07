import emacs

try:
    functionp = emacs.intern('functionp')
    assert functionp(emacs.intern('intern'))
    assert not functionp(emacs.intern('nil'))

    emacs.message('Tripoli successfully loaded')
except Exception as e:
    print('{}: {}'.format(e.__class__.__name__, e))

print('Tripoli successfully loaded')
