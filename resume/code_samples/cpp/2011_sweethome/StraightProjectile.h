#ifndef __SIMPLE_PROJECTILE_HEADER__INCLUDED__
#define __SIMPLE_PROJECTILE_HEADER__INCLUDED__

#include "RenderUtility\Projectile.h"
#include "RenderUtility\BaseTimeCounter.h"

class CStraightProjectile : public ISimpleProjectile
{
public:
	CStraightProjectile(D3DXVECTOR3 pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f), ICreature *pMaster = 0);
	virtual ~CStraightProjectile();
public:
	//virtuals from parents
	virtual DWORD AffectEnvironment(CLevelMap *pLevel, DWORD dwTime);
	virtual void EndProjectileOffence();
	virtual IRendItem* GetRendItem( ) const;
	virtual bool NeedsKilling() const;
	virtual DWORD Update(DWORD dwTime);
public:
	FLOAT GetAngle() const;
	FLOAT GetMaxDistance() const;
	FLOAT GetVelocity() const;

	void SetAngle(FLOAT dblRads);
	void SetAngleByPoints(D3DXVECTOR3 vStart, D3DXVECTOR3 vEnd);
	void SetMaxDistance(FLOAT dist = 0.0f);
	void SetTexture(LPWSTR szFile);
	void SetTimeToLive(DWORD ttl = 0);
	void SetVelocity(FLOAT velocity = 0.0f);
private:
	bool m_bNeedsKilling; //name says it all
	FLOAT m_dblAngle; //direction of the projectile
	FLOAT m_dblVelocity; //projectile velocity(px/sec)
	FLOAT m_dblDistance; //current distance from point of origination
	FLOAT m_dblMaxDistance; //maximum distance from point of origination
	CBasicSingleCounter ExecutionTimer; //microseconds
	LPWSTR m_szTexture; //texture file
};

#endif